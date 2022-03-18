//
// Created by 刘典 on 2021/9/12.
//

#ifndef NODEDRIVING_INVOCABLE_PARSER_HPP
#define NODEDRIVING_INVOCABLE_PARSER_HPP

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include "invocable.hpp"
#include <filesystem>
#include <list>
#include <optional>
#include "package_library.h"
#include <QJsonObject>
#include "utils.h"

class FindInvocableContext {
private:
    std::list<Invocable> &_result;
    const std::filesystem::path &_includePaths, &_file;
    std::string _package;
public:
    FindInvocableContext(std::list<Invocable> &result, const std::filesystem::path &includePaths,
                         const std::filesystem::path &file, std::string package) : _result(result),
                                                                                   _includePaths(includePaths),
                                                                                   _file(file), _package(package) {}

    FindInvocableContext(const FindInvocableContext &other) = delete;

    FindInvocableContext &operator=(const FindInvocableContext &other) = delete;


    std::list<Invocable> &getResult() const {
        return _result;
    }

    const std::filesystem::path &getIncludePaths() const {
        return _includePaths;
    }

    const std::filesystem::path &getFile() const {
        return _file;
    }

    std::filesystem::path getHeaderFile() const {
        return std::filesystem::relative(_file, _includePaths);
    };

    const std::string &getPackage() {
        return _package;
    }

};

class FindInvocableVisitor
        : public clang::RecursiveASTVisitor<FindInvocableVisitor> {
public:
    explicit FindInvocableVisitor(clang::ASTContext *context, FindInvocableContext &findInvocableContext)
            : _context(context), _findContext(findInvocableContext) {}


    bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
        if (!inFile(decl))
            return true;
        Invocable invocable;
        invocable.setType(Invocable::Class);
        invocable.setName(decl->getQualifiedNameAsString());
        invocable.setHeaderFile(_findContext.getHeaderFile().string());
        invocable.setPackage(_findContext.getPackage());
        for (const auto *method: decl->methods()) {
            const auto *ctor = clang::dyn_cast<clang::CXXConstructorDecl>(method);
            if (!ctor)
                continue;
            if (ctor->getAccess() != clang::AS_public)
                continue;
            if (ctor->isDefaultConstructor() || ctor->isCopyOrMoveConstructor())
                continue;
            parseConstructorParams(ctor, invocable);

        }
        if (parseFields(decl, invocable)) {
            _findContext.getResult().push_back(invocable);
        }

        return true;
    }
    bool VisitTypedefNameDecl(clang::TypedefNameDecl *decl) {
        if (!inFile(decl))
            return true;

        const auto *tst = decl->getTypeSourceInfo()->getType()->getAs<clang::TemplateSpecializationType>();
        std::string name = decl->getQualifiedNameAsString();
        if(!tst)
            return true;
        if(!tst->isRecordType())
            return true;
        const std::string & temp_name = tst->getAs<clang::RecordType>()->getDecl()->getQualifiedNameAsString();
        if (tst->getNumArgs() != 1)
            return {};
        if (tst->getArg(0).getKind() != clang::TemplateArgument::Type)
            return {};
        const std::string & arg_type = tst->getArg(0).getAsType().getAsString();

        Invocable invocable;
        Port port;
        port.setName("");
        port.setType(arg_type);
        if(temp_name == "adas::subsystem::in") {
            invocable.setType(Invocable::SubsystemIn);
            port.setDirection(Port::Out);
            invocable.setPortList({port});

        } else if(temp_name == "adas::subsystem::out") {
            invocable.setType(Invocable::SubsystemOut);
            port.setDirection(Port::In);
            invocable.setPortList({port});

        } else if(temp_name == "calibration::param") {
            invocable.setType(Invocable::CalibrationParam);
            port.setDirection(Port::Out);
            invocable.setPortList({port});
            Param param;
            param.setName("default");
            param.setType(arg_type);
            invocable.setParamList({param});


        } else
            return true;
        invocable.setName(name);
        invocable.setHeaderFile(_findContext.getHeaderFile().string());
        invocable.setPackage(_findContext.getPackage());
        _findContext.getResult().push_back(invocable);
        return true;
    }





private:
    std::optional<Port> parsePort(const clang::FieldDecl *field) {
        if (field->getAccess() != clang::AS_public)
            return {};
        Port ret;
        ret.setName(field->getNameAsString());
        clang::QualType type = field->getType();
        const auto *tst = type->getAs<clang::TemplateSpecializationType>();
        if (!tst)
            return {};
        if (!tst->isRecordType())
            return {};
        std::string temp_name = tst->getAs<clang::RecordType>()->getDecl()->getQualifiedNameAsString();
        if (temp_name == "adas::node::out")
            ret.setDirection(Port::Out);
        else if (temp_name == "adas::node::in")
            ret.setDirection(Port::In);
        else
            return {};
        if (tst->getNumArgs() != 1)
            return {};
        if (tst->getArg(0).getKind() != clang::TemplateArgument::Type)
            return {};
        ret.setType(tst->getArg(0).getAsType().getAsString());
        return ret;
    }

    bool parseFields(const clang::CXXRecordDecl *decl, Invocable &invocable) {
        std::vector<Port> ports;
        for (const auto *field: decl->fields()) {
            auto port_opt = parsePort(field);
            if (!port_opt)
                continue;
            ports.push_back(*port_opt);
        }
        if (ports.empty())
            return false;
        invocable.setPortList(ports);
        return true;
    }

    void parseConstructorParams(const clang::CXXConstructorDecl *decl, Invocable &invocable) {
        std::vector<Param> params(decl->getNumParams());
        for (int i = 0; i < decl->getNumParams(); ++i) {
            auto &p = params[i];
            const clang::ParmVarDecl *parmVarDecl = decl->getParamDecl(i);
            p.setType(parmVarDecl->getType().getAsString());
            p.setName(parmVarDecl->getNameAsString());
        }
        invocable.setParamList(params);

    }

    bool inFile(clang::Decl *decl) {
        clang::FullSourceLoc fullSourceLoc = _context->getFullLoc(decl->getBeginLoc());
        if (!fullSourceLoc.isValid())
            return false;
        auto file_entry = fullSourceLoc.getFileEntry();
        if (!file_entry || !file_entry->isValid())
            return false;
        std::filesystem::path file = file_entry->getName().str();
        return std::filesystem::equivalent(file, _findContext.getFile());
    }

private:
    clang::ASTContext *_context;
    FindInvocableContext &_findContext;
};

class FindInvocableConsumer : public clang::ASTConsumer {
public:
    explicit FindInvocableConsumer(clang::ASTContext *context, FindInvocableContext &findInvocableContext)
            : _visitor(context, findInvocableContext) {}

    void HandleTranslationUnit(clang::ASTContext &context) override {
        _visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    FindInvocableVisitor _visitor;
};

class FindInvocableAction : public clang::ASTFrontendAction {
public:
    explicit FindInvocableAction(FindInvocableContext &findInvocableContext) : _findContext(findInvocableContext) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance &compiler, llvm::StringRef inFile) override {
        return std::make_unique<FindInvocableConsumer>(&compiler.getASTContext(), _findContext);
    }

private:
    FindInvocableContext &_findContext;
};

class FindInvocableActionFactory : public clang::tooling::FrontendActionFactory {
public:
    explicit FindInvocableActionFactory(FindInvocableContext &findContext) : _findContext(findContext) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<FindInvocableAction>(_findContext);
    }

private:
    FindInvocableContext &_findContext;
};

class DiagnosticConsumer : public clang::DiagnosticConsumer {
public:
    void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel, const clang::Diagnostic &Info) override {
        clang::DiagnosticConsumer::HandleDiagnostic(DiagLevel, Info);
        if (Info.hasSourceManager()) {
            llvm::outs() << Info.getLocation().printToString(Info.getSourceManager()) << ": ";
        }
        switch (DiagLevel) {

            case clang::DiagnosticsEngine::Ignored:
                llvm::outs() << "ignored: ";
                break;
            case clang::DiagnosticsEngine::Note:
                llvm::outs() << "note: ";
                break;
            case clang::DiagnosticsEngine::Remark:
                llvm::outs() << "remark: ";
                break;
            case clang::DiagnosticsEngine::Warning:
                llvm::outs() << "warning: ";
                break;
            case clang::DiagnosticsEngine::Error:
                llvm::outs() << "error: ";
                break;
            case clang::DiagnosticsEngine::Fatal:
                llvm::outs() << "fatal: ";
                break;
        }
        llvm::SmallVector<char> str;
        Info.FormatDiagnostic(str);
        llvm::outs() << str << "\n";
    }

};

class InvocableParser {
private:
    std::filesystem::path _includePaths;

    bool parse(const PackageNode &node, const std::string &package,
               const std::vector<std::filesystem::path> &include_directories, std::list<Invocable> &result,
               std::string &error_message) {
        std::cout << "parse: " << package << std::endl;
        std::cout << "include_directories: " << std::endl;
        for(const auto &inc: include_directories)
            std::cout << inc << std::endl;
        const auto &file = node.header_file_path();
        FindInvocableContext findInvocableContext(result, node.include_directory, file, package);
        clang::tooling::FixedCompilationDatabase compilation(".", {});
        clang::tooling::ClangTool tool(compilation, {file.string()});
        tool.appendArgumentsAdjuster(
                getInsertArgumentAdjuster("-std=c++17",
                                          clang::tooling::ArgumentInsertPosition::END));
        for (const auto &inc: include_directories) {
            tool.appendArgumentsAdjuster(
                    getInsertArgumentAdjuster(
                            {"-I", inc.string()},
                            clang::tooling::ArgumentInsertPosition::END));
        }
//        tool.appendArgumentsAdjuster(
//                getInsertArgumentAdjuster(
//                        {"-I", "/opt/clang+llvm-12.0.1-x86_64-linux-gnu-ubuntu/lib/clang/12.0.1/include"},
                      //TODO:modify path
//                        {"-I", "/home/fc/clang+llvm-12.0.0-x86_64-linux-gnu-ubuntu-20.04/lib/clang/12.0.0/include"},
//                        clang::tooling::ArgumentInsertPosition::END));
        ///获得配置信息
        QJsonObject jo = getConfig();

        // tool.appendArgumentsAdjuster(
        //             getInsertArgumentAdjuster({"-I",jo.value("clang").toString().append("/include").toStdString()},
        //                                       clang::tooling::ArgumentInsertPosition::END));

        tool.appendArgumentsAdjuster(
                    getInsertArgumentAdjuster({"-I", "/home/environment/llvm12_ubuntu16/lib/clang/12.0.1/include"},
                                              clang::tooling::ArgumentInsertPosition::END));

#ifdef Q_OS_WIN64
        tool.appendArgumentsAdjuster(
                    getInsertArgumentAdjuster({"-I",jo.value("clangCXX").toString().toStdString()},
                                              clang::tooling::ArgumentInsertPosition::END));
        tool.appendArgumentsAdjuster(
                    getInsertArgumentAdjuster({"-I",jo.value("clangCXXAarch64").toString().toStdString()},
                                              clang::tooling::ArgumentInsertPosition::END));
#endif
        //        tool.appendArgumentsAdjuster(getInsertArgumentAdjuster("-v", // Verbose
        //                                                               clang::tooling::ArgumentInsertPosition::END));
        //        tool.appendArgumentsAdjuster(getInsertArgumentAdjuster("--language=c++", // C++
        //                                                               clang::tooling::ArgumentInsertPosition::END));
        FindInvocableActionFactory factory(findInvocableContext);
        DiagnosticConsumer dc;
        tool.setDiagnosticConsumer(&dc);
        int ret = tool.run(&factory);
        if (ret != 0) {
            error_message = "编译错误，请查看编译日志";
            return false;
        }
        return true;
    }


public:


    bool parse(const PackageLibrary &package_library, std::list<Invocable> &result, std::string &error_message) {
        for (const auto &p: package_library.packages()) {
            const auto &package_name = p.first;
            for (const auto &n: p.second.import.nodes) {

                if (!parse(n, package_name, package_library.package_include_directories(package_name), result,
                           error_message))
                    return false;
            }
        }
        return true;
    }


};


#endif //NODEDRIVING_INVOCABLE_PARSER_HPP
