//
// Created by liudian on 2022/3/9.
//

#include "subsystem_library.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <nodes/FlowScene>
#include <iostream>
#include <set>

void SubsystemLibrary::setPath(const std::filesystem::path &path) {
    path_ = path;

}

void SubsystemLibrary::setSystemPath(const std::filesystem::path &path) {
    systemPath_ = path;

}

bool SubsystemLibrary::hasSubsystem(const std::string &package, const std::string &name) const{
    return std::filesystem::exists(subsystemPath(package, name)) || std::filesystem::exists(systemSubsystemPath(package, name));
}

bool SubsystemLibrary::isSystemSubsystem(const std::string &package, const std::string &name) const {
    return std::filesystem::exists(systemSubsystemPath(package, name));
}

std::filesystem::path SubsystemLibrary::newSubsystem(const std::string &package, const std::string &name) {
    auto ret = subsystemPath(package, name);
    if(!std::filesystem::exists(ret.parent_path()))
        std::filesystem::create_directories(ret.parent_path());
    QtNodes::FlowScene scene;
    auto buffer = scene.saveToMemory();
    std::ofstream file(ret, std::ios::binary);
    file.write(buffer.data(), buffer.size());
    return ret;
}

std::filesystem::path SubsystemLibrary::getSubsystem(const std::string &package, const std::string &name) const {
    if(hasSubsystem(package, name)) {
        if(isSystemSubsystem(package, name))
            return systemSubsystemPath(package, name);
        else
            return subsystemPath(package, name);

    } else {
        return subsystemPath(package, name);
    }
}

std::filesystem::path SubsystemLibrary::subsystemPath(const std::string &package, const std::string &name) const {
    return  path_ / package / (name + ".flow");
}

std::filesystem::path SubsystemLibrary::systemSubsystemPath(const std::string &package, const std::string &name) const {
    return  systemPath_ / package / (name + ".flow");
}

std::vector<Invocable> SubsystemLibrary::getInvocableList() const {
    std::vector<Invocable> ret;
    std::set<std::string> names;
    if(path_.empty())
        return ret;
    for(const auto & dir_entry: std::filesystem::recursive_directory_iterator(path_)) {
        const auto & p = dir_entry.path();
        if(std::filesystem::is_regular_file(p) && p.has_extension() && p.extension() == ".flow") {
            Invocable invocable = invocableFromPath(p);
            ret.push_back(invocable);
            names.insert(invocable.getName());
        }
    }
    if(!systemPath_.empty()) {
        for(const auto & dir_entry: std::filesystem::recursive_directory_iterator(systemPath_)) {
            const auto & p = dir_entry.path();
            if(std::filesystem::is_regular_file(p) && p.has_extension() && p.extension() == ".flow") {
                Invocable invocable = invocableFromPath(p);
                if(names.find(invocable.getName()) != names.end())
                    continue;
                ret.push_back(invocable);
            }
        }

    }
    return ret;
}


boost::json::object SubsystemLibrary::readScene(const std::filesystem::path & path) {
    std::ifstream json_file{path};
    std::string json_text{std::istreambuf_iterator<char>(json_file), std::istreambuf_iterator<char>()};
    return boost::json::parse(json_text).as_object();
}

void SubsystemLibrary::parsePorts(boost::json::object &scene, Invocable &invocable) {
    std::vector<Port> ports;
    boost::json::value & nodes_value = scene["nodes"];
    if(!nodes_value.is_array())
        return;
    for(auto &node_value: nodes_value.as_array()) {
        boost::json::object & model = node_value.as_object()["model"].as_object();

        Port port;
        if(model["subsystem_in"].is_object()) {
            boost::json::object & subsystem_in  = model["subsystem_in"].as_object();
            port.setDirection(Port::In);
            std::string type = subsystem_in["type"].as_string().c_str();
            port.setType(type);
        } else if(model["subsystem_out"].is_object()) {
            boost::json::object & subsystem_out  = model["subsystem_out"].as_object();
            port.setDirection(Port::Out);
            std::string type = subsystem_out["type"].as_string().c_str();
            port.setType(type);
        } else {
            continue;
        }
        port.setName(model["var_name"].as_string().c_str());
        ports.push_back(port);
    }
    std::sort(ports.begin(), ports.end(), [](const Port & a, const Port & b){
        return a.getName() < b.getName();
    });
    invocable.setPortList(ports);
}

Invocable SubsystemLibrary::invocableFromPath(const std::filesystem::path &path) {
    Invocable invocable;
    invocable.setType(Invocable::Subsystem);
    invocable.setPackage(path.parent_path().filename().string());
    invocable.setSubsystemName(path.stem().string());
    invocable.setName(invocable.getPackage() + "::" + invocable.getSubsystemName());
    invocable.setHeaderFile(invocable.getPackage() + "/" + invocable.getSubsystemName() + ".hpp");
    boost::json::object scene = readScene(path);
    parsePorts(scene, invocable);
    return invocable;
}





