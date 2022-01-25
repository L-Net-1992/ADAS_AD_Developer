//
// Created by liudian on 2022/1/21.
//

#ifndef NODEDRIVING_PACKAGE_LIBRARY_H
#define NODEDRIVING_PACKAGE_LIBRARY_H

#include <vector>
#include <set>
#include <map>
#include <filesystem>
#include <utility>
#include <string>
#include <boost/json.hpp>

class PackageNode {
public:
    PackageNode(const std::filesystem::path &includeDirectory, const std::filesystem::path &headerFile);

    std::filesystem::path include_directory;
    std::filesystem::path header_file;
    std::filesystem::path header_file_path() const {
        return include_directory / header_file;
    }

};
class PackageImport {
public:
    std::vector<std::string> dependencies;
    std::vector<std::filesystem::path> include_directories;
    std::vector<PackageNode> nodes;


};
class CMakePackage {
private:
    std::pair<std::string, std::vector<std::string>> package_;
public:
    void name(const std::string & name) {
        package_.first = name;
    }
    void components(const std::vector<std::string> & components) {
        package_.second = components;
    }
    const std::string & name() const {
        return package_.first;
    }
    const std::vector<std::string> & components() const {
        return package_.second;
    }


};
class PackageBuild {
public:
    CMakePackage cmake_package;
    std::vector<std::string> libraries;

};
class Package {
public:
    std::string name;
    std::filesystem::path prefix_path;
    PackageImport import;
    PackageBuild build;

};
class PackageLibrary {
private:
    std::vector<std::filesystem::path> prefix_paths_;
    std::map<std::string, Package> packages_;
    void load_package(const std::filesystem::path & config);
    static void load_import(Package & package, const std::filesystem::path & config, boost::json::value & json);
    void load_build(Package & package, const std::filesystem::path & config, boost::json::value & json);
    static bool is_prefix(const std::filesystem::path & base, const std::filesystem::path & path);
    void package_include_directories(const std::string & package, std::vector<std::filesystem::path> & result, std::set<std::string> used) const;
public:
    const std::vector<std::filesystem::path> & prefix_paths() const { return prefix_paths_;};
    const std::map<std::string, Package> & packages() const {return packages_; };

    void add_prefix_path(const std::filesystem::path & path);
    void load_prefix_paths_from_config_file(const std::filesystem::path & config);
    std::vector<std::filesystem::path> package_include_directories(const std::string & package)const ;
    void clear() {
        packages_.clear();
        prefix_paths_.clear();
    }


};


#endif //NODEDRIVING_PACKAGE_LIBRARY_H
