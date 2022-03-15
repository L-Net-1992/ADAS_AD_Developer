//
// Created by liudian on 2022/3/9.
//

#include "subsystem_library.h"
#include <fstream>
#include "module_library.hpp"
#include <vector>
#include "models.hpp"
#include <nodes/Node>
#include <algorithm>
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
    if(!dataModelRegistry_ || dataModelRegistry_->categories().empty())
        return ret;
    if(path_.empty())
        return ret;
    QtNodes::FlowScene scene;
    scene.setRegistry(dataModelRegistry_);
    for(const auto & dir_entry: std::filesystem::recursive_directory_iterator(path_)) {
        const auto & p = dir_entry.path();
        if(std::filesystem::is_regular_file(p) && p.has_extension() && p.extension() == ".flow") {
            Invocable invocable;
            invocable.setType(Invocable::Subsystem);
            invocable.setPackage(p.parent_path().filename().string());
            invocable.setSubsystemName(p.stem().string());
            invocable.setName(invocable.getPackage() + "::" + invocable.getSubsystemName());
            scene.clearScene();
            readScene(scene, p);
            parsePorts(scene, invocable);
            ret.push_back(invocable);
        }

    }
    return ret;
}

SubsystemLibrary::SubsystemLibrary(ModuleLibrary *moduleLibrary):moduleLibrary_(moduleLibrary) {
    QObject::connect(moduleLibrary_, &ModuleLibrary::importCompleted, [this](){
        onImportCompleted();
    });

}

void SubsystemLibrary::readScene(QtNodes::FlowScene &scene, const std::filesystem::path & path) {
    int file_size = static_cast<int>(std::filesystem::file_size(path));
    QByteArray buffer(file_size, 0);
    std::ifstream file(path, std::ios::binary);
    file.read(buffer.data(), buffer.size());
    scene.loadFromMemory(buffer);
}

void SubsystemLibrary::parsePorts(const QtNodes::FlowScene &scene, Invocable &invocable) {
    std::vector<Port> ports;
    for(const auto *node: scene.allNodes()) {
        const auto* model = static_cast<const InvocableDataModel*>(node->nodeDataModel());
        const Invocable i = model->invocable();
        Port port;
        if(i.getType() == Invocable::SubsystemIn) {
            port.setDirection(Port::In);
        } else if(i.getType() == Invocable::SubsystemOut) {
            port.setDirection(Port::Out);
        } else {
            continue;
        }
        port.setType(i.getPortList().front().getType());
        port.setName(i.getVarName());
        ports.push_back(port);
    }
    std::sort(ports.begin(), ports.end(), [](const Port & a, const Port & b){
        return a.getName() < b.getName();
    });
    invocable.setPortList(ports);
}

void SubsystemLibrary::onImportCompleted() {
    dataModelRegistry_ = moduleLibrary_->test2NoSubsystem();
    if(path_.empty())
        return;
    for(const auto & dir_entry: std::filesystem::recursive_directory_iterator(path_)) {
        const auto & p = dir_entry.path();
        if(std::filesystem::is_regular_file(p) && p.has_extension() && p.extension() == ".flow") {
            Invocable invocable;
            invocable.setType(Invocable::Subsystem);
            invocable.setPackage(p.parent_path().filename().string());
            invocable.setSubsystemName(p.stem().string());
            invocable.setName(invocable.getPackage() + "::" + invocable.getSubsystemName());
            auto f = [invocable]() { return std::make_unique<InvocableDataModel>(invocable); };
            dataModelRegistry_->registerModel<InvocableDataModel>(f, "test");
        }

    }
}
