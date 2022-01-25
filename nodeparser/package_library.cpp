//
// Created by liudian on 2022/1/21.
//

#include "package_library.h"
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <iostream>

void PackageLibrary::add_prefix_path(const std::filesystem::path &path) {
    if(!path.is_absolute())
        throw std::runtime_error(path.generic_string() + " not a absolute path");
    if(!std::filesystem::is_directory(path)) {
        throw std::runtime_error(path.generic_string() + " not a directory");
    }
    auto share_path = path / "share";
    if(!std::filesystem::is_directory(share_path))
        return;
    for(const auto & e:std::filesystem::recursive_directory_iterator(share_path)) {
        if(!e.is_regular_file())
            continue;
        if(e.path().filename() != "adas-package.json")
            continue;
        if(e.path().parent_path().parent_path() != share_path)
            continue;
        if(!std::filesystem::exists(e.path()))
            continue;
        load_package(e.path());
    }

}

void PackageLibrary::load_package(const std::filesystem::path &config) {
//    qDebug() << "load package: " << config << std::endl;
    std::cout << "load package: " << config << std::endl;
    std::ifstream json_file{config.c_str()};
    std::string json_text{std::istreambuf_iterator<char>(json_file), std::istreambuf_iterator<char>()};
    boost::json::value json = boost::json::parse(json_text);
    Package package;
    package.name = config.parent_path().filename().string();
//    qInfo() << "package.name: " << package.name << std::endl;
    std::cout << "package.name: " << package.name << std::endl;
    package.prefix_path = config.parent_path().parent_path().parent_path();
    std::cout << "package.prefix: " << package.prefix_path << std::endl;
    if(!json.is_object()) {
        throw std::runtime_error(config.string() + " not a json object");
    }
    auto import = json.as_object()["import"];
    if(!import.is_null())
        load_import(package, config, import);
    auto build = json.as_object()["build"];
    if(!build.is_null())
        load_build(package, config, build);
    prefix_paths_.emplace_back(package.prefix_path);
    packages_.emplace(package.name, package);
}

bool PackageLibrary::is_prefix(const std::filesystem::path &base, const std::filesystem::path &path) {
    return std::search(path.begin(), path.end(), base.begin(), base.end()) == path.begin();
}

void PackageLibrary::load_build(Package &package, const std::filesystem::path & config, boost::json::value &json) {
    if(!json.is_object()) {
        throw std::runtime_error(config.string() + " build not a json object");
    }
    auto cmake = json.as_object()["cmake_package"];
    if(!cmake.is_null()) {
        if(!cmake.is_object())
            throw std::runtime_error(config.string() + " build.cmake_package not a json object");
        package.build.cmake_package.name(cmake.as_object()["name"].as_string().c_str());
    }
    auto libs = json.as_object()["libraries"];
    if(!libs.is_null()) {
        if(!libs.is_array())
            throw std::runtime_error(config.string() + " build.libraries not a json array");
        for(const auto & v:libs.as_array()) {
            package.build.libraries.emplace_back(v.as_string().c_str());
        }
    }


}

void PackageLibrary::load_import(Package &package, const std::filesystem::path & config, boost::json::value &json) {
    if(!json.is_object()) {
        throw std::runtime_error(config.string() + " import not a json object");
    }
    auto dep = json.as_object()["dependencies"];
    if(!dep.is_null()) {
        if(!dep.is_array())
            throw std::runtime_error(config.string() + " import.dependencies not a array");
        std::cout << "package.import.dependencies: ";
        for(const auto &v: dep.as_array()) {
            package.import.dependencies.emplace_back(v.as_string().c_str());
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }
    auto incs = json.as_object()["include_directories"];
    if(!incs.is_null()) {
        if(!incs.is_array())
            throw std::runtime_error(config.string() + " import.include_directories not a array");
        std::cout << "package.import.include_directories: ";
        for(const auto &v: incs.as_array()) {
            std::filesystem::path  inc(v.as_string().c_str());
            if(inc.is_relative())
                inc = package.prefix_path / inc;
            package.import.include_directories.emplace_back(inc);
            std::cout << inc << " ";
        }
        std::cout << std::endl;
    }
    auto nodes = json.as_object()["nodes"];
    if(!nodes.is_null()) {
        if(!nodes.is_array())
            throw std::runtime_error(config.string() + " import.nodes not a array");
        for(const auto &v: nodes.as_array()) {
            std::filesystem::path  node_path(v.as_string().c_str());
            if(node_path.is_relative())
                node_path = package.prefix_path / node_path;
            for(const auto &e: std::filesystem::recursive_directory_iterator(node_path)) {
                const std::filesystem::path &  node = e.path();
                if(node.extension() != ".hpp")
                    continue;

                for(const auto & inc: package.import.include_directories) {
                    if(is_prefix(inc, node)) {
                        auto header_file = std::filesystem::relative(node, inc);
                        package.import.nodes.emplace_back(PackageNode{inc, header_file});
                        std::cout << "node include: " << inc << " header_file: " << header_file << std::endl;
                        break;
                    }
                }

            }
        }
    }

}

void PackageLibrary::load_prefix_paths_from_config_file(const std::filesystem::path &config) {
    if(!config.is_absolute())
        throw std::runtime_error(config.generic_string() + " not a absolute path");
    auto config_dir = config.parent_path();
    std::ifstream json_file{config.c_str()};
    std::string json_text{std::istreambuf_iterator<char>(json_file), std::istreambuf_iterator<char>()};
    boost::json::value json = boost::json::parse(json_text);
    if(!json.is_object()) {
        throw std::runtime_error(config.string() + " not a json object");
    }
    auto prefix = json.as_object()["prefix_paths"].as_array();
    for(const auto & elem: prefix) {
        std::filesystem::path prefix_path(elem.as_string().c_str());
        if(prefix_path.is_relative())
            prefix_path = config_dir / prefix_path;
        add_prefix_path(prefix_path);
    }


}

std::vector<std::filesystem::path> PackageLibrary::package_include_directories(const std::string &package) const {
    std::vector<std::filesystem::path> ret;
    std::set<std::string> used;
    package_include_directories(package, ret, used);
    return ret;
}


void PackageLibrary::package_include_directories(const std::string &package, std::vector<std::filesystem::path> &result,
                                            std::set<std::string> used) const {
    if(used.find(package) != used.end())
        return;
    const auto & p = packages_.at(package);
    for(const auto & inc:p.import.include_directories) {
        if(std::find(result.begin(), result.end(), inc) == result.end())
            result.emplace_back(inc);
    }
    used.emplace(package);
    for(const auto & dep: p.import.dependencies)
        package_include_directories(dep, result, used);
}

PackageNode::PackageNode(const std::filesystem::path &includeDirectory, const std::filesystem::path &headerFile)
        : include_directory(includeDirectory), header_file(headerFile) {}
