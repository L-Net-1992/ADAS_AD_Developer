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

class SourceGenerator {
private:
    static void extractIncludes(const QtNodes::FlowScene &scene, std::set<std::string> &includes) {
        for (const auto *node: scene.allNodes()) {
            const auto *model = static_cast<InvocableDataModel *>(node->nodeDataModel());
            includes.insert(model->invocable().getHeaderFile());
        }
    }

    static void
    makeNodeVarNames(const QtNodes::FlowScene &scene, std::map<const InvocableDataModel *, std::string> &nodeVarNames) {
        int i = 0;
        for (const auto *node: scene.allNodes()) {
            const auto *model = static_cast<InvocableDataModel *>(node->nodeDataModel());
            nodeVarNames.insert({model, "_node" + std::to_string(i++)});
        }
    }

    static void makeConnections(const QtNodes::FlowScene &scene,
                                const std::map<const InvocableDataModel *, std::string> &nodeVarNames,
                                std::map<std::string, std::vector<std::string>> &connections) {
        for(const auto & p: scene.connections()) {
            const auto & c = *p.second;
            const QtNodes::Node * out_node = c.getNode(QtNodes::PortType::Out);
            const auto *out_model = static_cast<InvocableDataModel *>(out_node->nodeDataModel());
            const QtNodes::Node * in_node = c.getNode(QtNodes::PortType::In);
            const auto *in_model = static_cast<InvocableDataModel *>(in_node->nodeDataModel());
            const auto & out_port = c.dataType(QtNodes::PortType::Out);
            const auto & in_port = c.dataType(QtNodes::PortType::In);
            std::string out = nodeVarNames.find(out_model)->second + "." + out_port.name.toStdString();
            std::string in = nodeVarNames.find(in_model)->second + "." + in_port.name.toStdString();
            if(connections.find(out) == connections.end())
                connections.insert({out, {}});
            connections.find(out)->second.push_back(in);


        }

    }

    static std::string indent(int n) {
        return std::string(4 * n, ' ');
    }

    static void generateIncludes(const std::set<std::string> &includes, std::ofstream &file) {
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
    static void generateTask(const std::map<const InvocableDataModel *, std::string> &varNames,
                             const std::map<std::string, std::vector<std::string>> &connections, std::ofstream &file) {
        file << "class task0 {" << std::endl;
        file << "private:" << std::endl;
        for(const auto & p: varNames) {
            const std::string & type = p.first->invocable().getName();
            const std::string & name = p.second;
            file << indent(1) << type << " " << name << nodeParamValueList(p.first->invocable().getParamList()) << ";" << std::endl;
        }
        file << "public:" << std::endl;
        file << indent(1) << "task0() {" << std::endl;
        for(const auto & p: connections) {
            file << indent(2) << p.first << ".handler([this](auto value) {" << std::endl;
            for(const auto & in: p.second) {
                file << indent(3) << in << ".set(value);" << std::endl;
            }
            file << indent(2) <<"});" << std::endl;

        }
        file << indent(1) << "}" << std::endl;
        file << "};" << std::endl;

    }
    static void generateMain(std::ofstream & file) {
        file << "int main(int argc, char **argv) {" << std::endl;
        file << indent(1) << "adas::runtime::init(argc, argv);" << std::endl;
        file << indent(1) << "adas::runtime::task<task0> _task0;" << std::endl;
        file << indent(1) << "pause();" << std::endl;
        file << "}" << std::endl;

    }

    static std::set<std::string> linkedPackage(const QtNodes::FlowScene &scene) {
        std::set<std::string> ret;
        for (const auto *node: scene.allNodes()) {
            const auto *model = static_cast<InvocableDataModel *>(node->nodeDataModel());
            ret.emplace(model->invocable().getPackage());
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
        std::set<std::string> includes;
        std::map<const InvocableDataModel *, std::string> varNames;
        std::map<std::string, std::vector<std::string>> connections;
        extractIncludes(scene, includes);
        makeNodeVarNames(scene, varNames);
        makeConnections(scene, varNames, connections);
        generateIncludes(includes, file);
        generateTask(varNames, connections, file);
        generateMain(file);

    }
    static void generateCMakeList(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::string & source ,std::ofstream &file) {
        file << "cmake_minimum_required(VERSION 3.10)" << std::endl;
        file << "project(adas_generate)" << std::endl;
        file << "set(CMAKE_CXX_STANDARD 17)" << std::endl;
        file << "find_package(Threads REQUIRED)" << std::endl;
        const auto & linked = linkedPackage(scene);
        const auto & libs = librariesFromPackages(packageLibrary, linked);
        for(const auto & name: linked) {
            const auto & p = packageLibrary.packages().at(name);
            if(p.build.cmake_package.name().empty())
                continue;
            file << "find_package(" << p.build.cmake_package.name() << " REQUIRED)" << std::endl;
        }
        file << "add_executable(adas_generate " << source << ")" << std::endl;
        file << "target_link_libraries(adas_generate Threads::Threads";
        for(const auto & lib:libs) {
            file << " " << lib;
        }
        file << ")" << std::endl;
    }
    static void generateCMakeProject(const std::filesystem::path & directory, const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary) {
        std::ofstream source(directory / "generate.cpp");
        std::ofstream cmakeList(directory / "CMakeLists.txt");
        generateSource(scene, source);
        generateCMakeList(scene, packageLibrary, "generate.cpp", cmakeList);

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
        file << "echo 'Build Completed'" << std::endl;

    }
    static void generateDeployScript(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::filesystem::path & config_file ,std::ofstream &file) {
        auto config = loadConfig(config_file);
        file << "#!/usr/bin/env bash" << std::endl;
        file << "SRC_DIR=$(dirname $(realpath \"$0\"))" << std::endl;
        file << "cd $SRC_DIR" << std::endl;
        file << config["kill_cmd"].as_string().c_str() << std::endl;
        file << config["deploy_cmd"].as_string().c_str() << std::endl;
        file << "echo 'Deploy Completed'" << std::endl;

    }
    static void generateRunScript(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::filesystem::path & config_file ,std::ofstream &file) {
        auto config = loadConfig(config_file);
        file << "#!/usr/bin/env bash" << std::endl;
        file << "SRC_DIR=$(dirname $(realpath \"$0\"))" << std::endl;
        file << "cd $SRC_DIR" << std::endl;
        file << config["kill_cmd"].as_string().c_str() << std::endl;
        file << "echo 'Start Running'" << std::endl;
        file << config["run_cmd"].as_string().c_str() << std::endl;

    }
    static void generateStopScript(const QtNodes::FlowScene &scene, const PackageLibrary & packageLibrary, const std::filesystem::path & config_file ,std::ofstream &file) {
        auto config = loadConfig(config_file);
        file << "#!/usr/bin/env bash" << std::endl;
        file << "SRC_DIR=$(dirname $(realpath \"$0\"))" << std::endl;
        file << "cd $SRC_DIR" << std::endl;
        file << config["kill_cmd"].as_string().c_str() << std::endl;
        file << "echo 'Stop'" << std::endl;

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
