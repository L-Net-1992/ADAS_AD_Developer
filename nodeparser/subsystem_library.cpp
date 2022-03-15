//
// Created by liudian on 2022/3/9.
//

#include "subsystem_library.h"
#include <nodes/FlowScene>
#include <fstream>
void SubsystemLibrary::setPath(const std::filesystem::path &path) {
    path_ = path;

}

bool SubsystemLibrary::hasSubsystem(const std::string &package, const std::string &name) {
    return std::filesystem::exists(subsystemPath(package, name));

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

std::filesystem::path SubsystemLibrary::getSubsystem(const std::string &package, const std::string &name) {
    return subsystemPath(package, name);
}

std::filesystem::path SubsystemLibrary::subsystemPath(const std::string &package, const std::string &name) {
    return  path_ / package / (name + ".flow");
}

std::vector<Invocable> SubsystemLibrary::getInvocableList() {
    std::vector<Invocable> ret;
    for(const auto & dir_entry: std::filesystem::recursive_directory_iterator(path_)) {
        const auto & p = dir_entry.path();
        if(std::filesystem::is_regular_file(p) && p.has_extension() && p.extension() == ".flow") {
            Invocable invocable;
            invocable.setType(Invocable::Subsystem);
            invocable.setPackage(p.parent_path().filename().string());
            invocable.setSubsystemName(p.stem().string());
            invocable.setName(invocable.getPackage() + "::" + invocable.getSubsystemName());
            ret.push_back(invocable);
        }

    }
    return ret;
}
