//
// Created by 刘典 on 2021/10/21.
//

#ifndef NODEDRIVING_SOURCE_GENERATOR_HPP
#define NODEDRIVING_SOURCE_GENERATOR_HPP

#include <nodes/FlowScene>
#include <nodes/Node>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include "models.hpp"
#include <iostream>
#include <sstream>
#include <boost/json.hpp>
#include "package_library.h"
#include "subsystem_library.h"
#include "module_library.hpp"

class SourceGenerator {
private:
    static void extractIncludes(const QtNodes::FlowScene &scene, std::set<std::string> &includes) {
        for (const auto *node: scene.allNodes()) {
            const auto *model = static_cast<InvocableDataModel *>(node->nodeDataModel());
            includes.insert(model->invocable().getHeaderFile());
        }
    }

//    static void
//    makeNodeVarNames(const QtNodes::FlowScene &scene, std::map<const InvocableDataModel *, std::string> &nodeVarNames) {
//        int i = 0;
//        for (const auto *node: scene.allNodes()) {
//            const auto *model = static_cast<InvocableDataModel *>(node->nodeDataModel());
//            nodeVarNames.insert({model, "_node" + std::to_string(i++)});
//        }
//    }

    static void makeConnections(const QtNodes::FlowScene &scene,
                                std::map<std::string, std::vector<std::string>> &connections) {
        for(const auto & p: scene.connections()) {
            const auto & c = *p.second;
            const QtNodes::Node * out_node = c.getNode(QtNodes::PortType::Out);
            const auto *out_model = static_cast<InvocableDataModel *>(out_node->nodeDataModel());
            if(out_model->invocable().getType() != Invocable::Class && out_model->invocable().getType() != Invocable::Subsystem)
                continue;
            const QtNodes::Node * in_node = c.getNode(QtNodes::PortType::In);
            const auto *in_model = static_cast<InvocableDataModel *>(in_node->nodeDataModel());
            const auto & out_port = c.dataType(QtNodes::PortType::Out);
            const auto & in_port = c.dataType(QtNodes::PortType::In);
            std::string out = out_model->invocable().getVarName() + "." + out_port.name.toStdString();
            if(connections.find(out) == connections.end())
                connections.insert({out, {}});
            if(in_model->invocable().getType() == Invocable::Class || in_model->invocable().getType() == Invocable::Subsystem) {
                std::string in = in_model->invocable().getVarName() + "." + in_port.name.toStdString() + ".set(value)";
                connections.find(out)->second.push_back(in);
            } else if(in_model->invocable().getType() == Invocable::SubsystemOut) {
                const std::string & arg_type = in_model->invocable().getInputPort(0).getType();
                std::string pl = "(value)";
                if(arg_type == "void") {
                    pl = "()";
                }
                std::string in = in_model->invocable().getVarName() + pl;
                connections.find(out)->second.push_back(in);
            }
        }
    }

    static std::string indent(int n) {
        return std::string(4 * n, ' ');
    }

    static void generateIncludes(const QtNodes::FlowScene &scene, std::ofstream &file) {
        std::set<std::string> includes;
        extractIncludes(scene, includes);
        file << "#include <adas/runtime.hpp>" << std::endl;
        file << "#include <unistd.h>" << std::endl;
        for (const auto &inc: includes)
            file << "#include <" << inc << ">" << std::endl;
    }
    static std::string nodeParamValueList(const std::vector<Param> & params) {
        if(params.empty())
            return "";
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for(const Param & p: params) {
            if(!first)
                ss << ", ";
            ss << p.getLiteral();
            first = false;
        }
        ss << "}";
        return ss.str();

    }

    static void generateNodeVar(const QtNodes::FlowScene &scene, std::ofstream &file) {
        for(const auto  node: scene.allNodes()) {
            const auto *model = static_cast<const InvocableDataModel*>(node->nodeDataModel());
            const std::string & type = model->invocable().getName();
            const std::string & name = model->invocable().getVarName();
            if(model->invocable().getType() == Invocable::Class) {
                file << indent(1) << type << " " << name << nodeParamValueList(model->invocable().getParamList()) << ";" << std::endl;
            }
            else if(model->invocable().getType() == Invocable::Subsystem) {
                file << indent(1) << type << " " << name << "{_name + \"." << name <<  "\"};" << std::endl;
            }
        }
    }
    static void makeCalibrationParamConnections(const QtNodes::FlowScene &scene,
                                                std::map<std::string, std::vector<std::string>> &connections) {
        for(const auto & p: scene.connections()) {
            const auto & c = *p.second;
            const QtNodes::Node * out_node = c.getNode(QtNodes::PortType::Out);
            const auto *out_model = static_cast<InvocableDataModel *>(out_node->nodeDataModel());
            if(out_model->invocable().getType() != Invocable::CalibrationParam)
                continue;
            const QtNodes::Node * in_node = c.getNode(QtNodes::PortType::In);
            const auto *in_model = static_cast<InvocableDataModel *>(in_node->nodeDataModel());
            const auto & out_port = c.dataType(QtNodes::PortType::Out);
            const auto & in_port = c.dataType(QtNodes::PortType::In);
            std::string out = out_model->invocable().getVarName();
            if(connections.find(out) == connections.end())
                connections.insert({out, {}});
            if(in_model->invocable().getType() == Invocable::Class || in_model->invocable().getType() == Invocable::Subsystem) {
                std::string in = in_model->invocable().getVarName() + "." + in_port.name.toStdString() + ".set(value)";
                connections.find(out)->second.push_back(in);
            } else if(in_model->invocable().getType() == Invocable::SubsystemOut) {
                std::string in = in_model->invocable().getVarName() + "(value)";
                connections.find(out)->second.push_back(in);
            }


        }

    }

    static void generateCalibrationParamConnections(const QtNodes::FlowScene &scene, std::ofstream &file) {
        //adas::runtime::this_task::context().calibration().on_update("", 0.0, [this](float value){});
        std::map<std::string, std::vector<std::string>> connections;
        makeCalibrationParamConnections(scene, connections);
        for(const auto & p: connections) {
            file << indent(2) << "adas::runtime::this_task::context().calibration().on_update(_name+\"."  << p.first << "\", 0.0, [this](float value) {" << std::endl;
            for(const auto & in: p.second) {
                file << indent(3) << in << ";" << std::endl;
            }
            file << indent(2) <<"});" << std::endl;

        }
    }
    static void generateConnections(const QtNodes::FlowScene &scene, std::ofstream &file) {
        //adas::runtime::this_task::context().inspect().register_var("");
        //adas::runtime::this_task::context().inspect().update("", 0.0f);
        std::map<std::string, std::vector<std::string>> connections;
        makeConnections(scene, connections);
        for(const auto & p: connections) {
            file << indent(2) << "adas::runtime::this_task::context().inspect().register_var(_name+\"."<< p.first <<"\");" << std::endl;
            file << indent(2) << p.first << ".handler([this](auto value) {" << std::endl;
            file << indent(3) << "adas::runtime::this_task::context().inspect().update(_name+\"."<< p.first <<"\", value);" << std::endl;
            for(const auto & in: p.second) {
                file << indent(3) << in << ";" << std::endl;
            }
            file << indent(2) <<"});" << std::endl;

        }
    }

    static void
    generateConstructor(const QtNodes::FlowScene &scene, const std::string &className, std::ofstream &file) {
        file << indent(1) << "explicit  " << className << "(const std::string & name):_name{name} {" << std::endl;
        generateCalibrationParamConnections(scene, file);
        generateConnections(scene, file);
        file << indent(1) << "}" << std::endl;
    }

    static void makeSubsystemInConnections(const QtNodes::FlowScene &scene,
                                std::map<std::string, std::vector<std::string>> &connections) {
        for(const auto & p: scene.connections()) {
            const auto & c = *p.second;
            const QtNodes::Node * out_node = c.getNode(QtNodes::PortType::Out);
            const auto *out_model = static_cast<InvocableDataModel *>(out_node->nodeDataModel());
            if(out_model->invocable().getType() != Invocable::SubsystemIn)
                continue;
            const QtNodes::Node * in_node = c.getNode(QtNodes::PortType::In);
            const auto *in_model = static_cast<InvocableDataModel *>(in_node->nodeDataModel());
            const auto & out_port = c.dataType(QtNodes::PortType::Out);
            const auto & in_port = c.dataType(QtNodes::PortType::In);
            std::string out = out_model->invocable().getVarName();
            if(connections.find(out) == connections.end())
                connections.insert({out, {}});
            if(in_model->invocable().getType() == Invocable::Class || in_model->invocable().getType() == Invocable::Subsystem) {
                std::string in = in_model->invocable().getVarName() + "." + in_port.name.toStdString() + ".set(value)";
                connections.find(out)->second.push_back(in);
            } else if(in_model->invocable().getType() == Invocable::SubsystemOut) {
                std::string in = in_model->invocable().getVarName() + "(value)";
                connections.find(out)->second.push_back(in);
            }


        }

    }
    static void generateSubsystemPort(const QtNodes::FlowScene &scene, std::ofstream &file) {
        std::map<std::string, std::vector<std::string>> connections;
        makeSubsystemInConnections(scene, connections);
        for(const auto  node: scene.allNodes()) {
            const auto *model = static_cast<const InvocableDataModel*>(node->nodeDataModel());
            const std::string & name = model->invocable().getVarName();
            if(model->invocable().getType() == Invocable::SubsystemIn) {
                std::string type = model->invocable().getOutputPort(0).getType();
                if(type == "_Bool")
                    type = "bool";
                file << indent(1) << "adas::subsystem::in<" << type << "> " << name << "{[this](const auto & value){"<< std::endl;
                for(const auto & in: connections.at(name)) {
                        file << indent(2) << in << ";" << std::endl;
                }

                file << indent(1) << "}};" << std::endl;
            }
            else if(model->invocable().getType() == Invocable::SubsystemOut) {
                std::cout << "num of input: " << model->invocable().getNumInput() << ", " << model->invocable().getInputPort(0).getType() << std::endl;
                std::string type = model->invocable().getInputPort(0).getType();
                if(type == "_Bool")
                    type = "bool";
                file << indent(1) << "adas::node::out<" << type << "> " << name << ";" << std::endl;
            }
        }


    }
    static void generateClass(const QtNodes::FlowScene &scene, const std::string & className, std::ofstream &file) {
        file << "class " << className << " {" << std::endl;
        file << "private:" << std::endl;
        file << indent(1) << "std::string _name;" << std::endl;
        generateNodeVar(scene, file);
        file << "public:" << std::endl;
        generateSubsystemPort(scene, file);
        file << "public:" << std::endl;
        generateConstructor(scene, className, file);
        file << "};" << std::endl;

    }

    static void generateNamespaceClass(const QtNodes::FlowScene &scene, const std::string & ns, const std::string & className, std::ofstream &file) {
        generateIncludes(scene, file);
        file << "namespace " << ns << "{" << std::endl;
        generateClass(scene, className, file);
        file << "}" << std::endl;

    }
    static void generateMain(std::ofstream & file) {
        file << "int main(int argc, char **argv) {" << std::endl;
        file << indent(1) << "adas::runtime::init(argc, argv);" << std::endl;
        file << indent(1) << "adas::runtime::task<task0> _task0{\"task0\"};" << std::endl;
        file << indent(1) << "pause();" << std::endl;
        file << "}" << std::endl;

    }

    static std::set<std::string> linkedPackage(const QtNodes::FlowScene &scene, const ModuleLibrary & moduleLibrary) {
        std::set<std::string> ret;
        for (const auto *node: scene.allNodes()) {
            const auto *model = static_cast<InvocableDataModel *>(node->nodeDataModel());
            if(model->invocable().getType() == Invocable::Class) {
                ret.emplace(model->invocable().getPackage());
            } else if(model->invocable().getType() == Invocable::Subsystem) {
                ret.emplace("adas_subsystem");
                QtNodes::FlowScene subScene(moduleLibrary.test2());
                loadScene(subScene, moduleLibrary.subsystemLibrary().getSubsystem(
                        model->invocable().getPackage(), model->invocable().getSubsystemName()));
                std::set<std::string> subPkg = linkedPackage(subScene, moduleLibrary);
                ret.insert(subPkg.begin(), subPkg.end());
            }
        }
        return ret;
    }
    static std::set<std::string> librariesFromPackages(const PackageLibrary & packageLibrary, const std::set<std::string> & packages) {
        std::set<std::string> ret;
        for(const auto & name: packages) {
            const auto & p = packageLibrary.packages().at(name);
            ret.insert(p.build.libraries.begin(), p.build.libraries.end());
        }
        return ret;
    }
    static boost::json::object loadConfig(const std::filesystem::path & config) {
        std::ifstream json_file{config.c_str()};
        std::string json_text{std::istreambuf_iterator<char>(json_file), std::istreambuf_iterator<char>()};
        return boost::json::parse(json_text).as_object();
    }
    static std::string packagesPrefix(const PackageLibrary & packageLibrary) {
        std::string ret;
        std::set<std::string> exists;
        for(const auto &p: packageLibrary.packages()) {
            std::string prefix = p.second.prefix_path.string();
            if(exists.find(prefix) != exists.end())
                continue;
            exists.emplace(prefix);
            if(ret.empty()) {
                ret = prefix;
            } else {
                ret += ";" + prefix;
            }
        }
        return "'" + ret + "'";
    }
public:
    static void generateSource(const QtNodes::FlowScene &scene, std::ofstream &file) {
        generateIncludes(scene, file);
        generateClass(scene, "task0", file);
        generateMain(file);

    }
    static void generateCMakeList(const QtNodes::FlowScene &scene, const ModuleLibrary & moduleLibrary, const std::string & source ,std::ofstream &file) {
        file << "cmake_minimum_required(VERSION 3.10)" << std::endl;
        file << "project(adas_generate)" << std::endl;
        file << "set(CMAKE_CXX_STANDARD 17)" << std::endl;
        file << "find_package(Threads REQUIRED)" << std::endl;
        const auto & linked = linkedPackage(scene, moduleLibrary);
        const auto & libs = librariesFromPackages(moduleLibrary.packageLibrary(), linked);
        for(const auto & name: linked) {
            const auto & p = moduleLibrary.packageLibrary().packages().at(name);
            if(p.build.cmake_package.name().empty())
                continue;
            file << "find_package(" << p.build.cmake_package.name() << " REQUIRED)" << std::endl;
        }
        file << "add_executable(adas_generate " << source << ")" << std::endl;
        file << "target_include_directories(adas_generate PRIVATE subsystem)" << std::endl;
        file << "target_link_libraries(adas_generate Threads::Threads";
        for(const auto & lib:libs) {
            file << " " << lib;
        }
        file << ")" << std::endl;
    }
    static void loadScene(QtNodes::FlowScene & scene, const std::filesystem::path & path) {
        int file_size = static_cast<int>(std::filesystem::file_size(path));
        QByteArray buffer(file_size, 0);
        std::ifstream file(path, std::ios::binary);
        file.read(buffer.data(), buffer.size());
        scene.loadFromMemory(buffer);
    }
    static void generateSubsystem(const std::filesystem::path & root, const Invocable &invocable, const ModuleLibrary & moduleLibrary) {
        std::filesystem::path dir = root / invocable.getPackage();
        if(!std::filesystem::exists(dir))
            std::filesystem::create_directories(dir);
        std::filesystem::path header_file = dir / (invocable.getSubsystemName() + ".hpp");
        if(std::filesystem::exists(header_file))
            return;
        std::ofstream header(header_file);
        QtNodes::FlowScene scene(moduleLibrary.test2());
        loadScene(scene, moduleLibrary.subsystemLibrary().getSubsystem(invocable.getPackage(), invocable.getSubsystemName()));
        header << "#pragma once" << std::endl;
        generateNamespaceClass(scene, invocable.getPackage(), invocable.getSubsystemName(), header);
        header.close();
        generateSubsystemInScene(root, scene, moduleLibrary);


    }
    static void generateSubsystemInScene(const std::filesystem::path & directory, const QtNodes::FlowScene &scene, const ModuleLibrary & moduleLibrary) {
        for(const auto *node: scene.allNodes()) {
            const auto *model = static_cast<InvocableDataModel *>(node->nodeDataModel());
            if(model->invocable().getType() == Invocable::Subsystem)
                generateSubsystem(directory, model->invocable(), moduleLibrary);
        }


    }
    static void generateCMakeProject(const std::filesystem::path & directory, const QtNodes::FlowScene &scene, const ModuleLibrary & moduleLibrary) {
        std::filesystem::path subsystem_dir = directory / "subsystem";

        if(std::filesystem::exists(subsystem_dir)) {
            std::filesystem::remove_all(subsystem_dir);
        }
        std::filesystem::create_directories(subsystem_dir);
        generateSubsystemInScene(subsystem_dir, scene, moduleLibrary);
        std::ofstream source(directory / "generate.cpp");
        generateSource(scene, source);
        std::ofstream cmakeList(directory / "CMakeLists.txt");
        generateCMakeList(scene, moduleLibrary, "generate.cpp", cmakeList);

    }
    static void generateBuildScript(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::filesystem::path & config_file ,std::ofstream &file) {
        auto config = loadConfig(config_file);
        file << "#!/usr/bin/env bash" << std::endl;
        file << "SRC_DIR=$(dirname $(realpath \"$0\"))" << std::endl;
        file << "BIN_DIR=$SRC_DIR/build_" << config["name"].as_string().c_str() << std::endl;
        file << "mkdir -p $BIN_DIR" << std::endl;
        std::string cmd =  "cd $BIN_DIR;rm -f CMakeCache.txt";
        auto toolchain = config["cmake_toolchain_file"];
        if(toolchain.is_null()) {
            cmd+= ";cmake -DCMAKE_PREFIX_PATH=" + packagesPrefix(packageLibrary);
        } else {
            cmd+= std::string(";cmake -DCMAKE_TOOLCHAIN_FILE=") + toolchain.as_string().c_str() + " -DCMAKE_FIND_ROOT_PATH=" + packagesPrefix(packageLibrary);
        }
        cmd += " $SRC_DIR;make";
        auto build_executor = config["build_executor"];
        if(!build_executor.is_null()) {
            std::string be = build_executor.as_string().c_str();
            cmd = be + " \"" + cmd + '"';
        }
        file << cmd << std::endl;

    }
    static void generateDeployScript(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::filesystem::path & config_file ,std::ofstream &file) {
        auto config = loadConfig(config_file);
        file << "#!/usr/bin/env bash" << std::endl;
        file << "SRC_DIR=$(dirname $(realpath \"$0\"))" << std::endl;
        file << "cd $SRC_DIR" << std::endl;
        file << config["kill_cmd"].as_string().c_str() << std::endl;
        file << config["deploy_cmd"].as_string().c_str() << std::endl;

    }
    static void generateRunScript(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::filesystem::path & config_file ,std::ofstream &file) {
        auto config = loadConfig(config_file);
        file << "#!/usr/bin/env bash" << std::endl;
        file << "SRC_DIR=$(dirname $(realpath \"$0\"))" << std::endl;
        file << "cd $SRC_DIR" << std::endl;
        file << config["kill_cmd"].as_string().c_str() << std::endl;
        file << config["run_cmd"].as_string().c_str() << std::endl;

    }
    static void generateStopScript(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::filesystem::path & config_file ,std::ofstream &file) {
        auto config = loadConfig(config_file);
        file << "#!/usr/bin/env bash" << std::endl;
        file << "SRC_DIR=$(dirname $(realpath \"$0\"))" << std::endl;
        file << "cd $SRC_DIR" << std::endl;
        file << config["kill_cmd"].as_string().c_str() << std::endl;

    }

    static void generateScript(const std::filesystem::path & directory, const std::filesystem::path & config_file, const std::string & name, const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary) {
        std::ofstream buildScriptJetson(directory / ("build_" + name + ".sh"));
        SourceGenerator::generateBuildScript(scene, packageLibrary, config_file, buildScriptJetson);
        std::filesystem::permissions(directory / ("build_" + name + ".sh"),std::filesystem::perms::owner_exec ,std::filesystem::perm_options::add);

        std::ofstream deployScriptJetson(directory / ("deploy_" + name + ".sh"));
        SourceGenerator::generateDeployScript(scene, packageLibrary, config_file, deployScriptJetson);
        std::filesystem::permissions(directory / ("deploy_" + name + ".sh"),std::filesystem::perms::owner_exec ,std::filesystem::perm_options::add);

        std::ofstream runScriptJetson(directory / ("run_" + name + ".sh"));
        SourceGenerator::generateRunScript(scene, packageLibrary, config_file, runScriptJetson);
        std::filesystem::permissions(directory / ("run_" + name + ".sh"),std::filesystem::perms::owner_exec ,std::filesystem::perm_options::add);

        std::ofstream stopScriptJetson(directory / ("stop_" + name + ".sh"));
        SourceGenerator::generateStopScript(scene, packageLibrary, config_file, stopScriptJetson);
        std::filesystem::permissions(directory / ("stop_" + name + ".sh"),std::filesystem::perms::owner_exec ,std::filesystem::perm_options::add);

    }

};

#endif //NODEDRIVING_SOURCE_GENERATOR_HPP
