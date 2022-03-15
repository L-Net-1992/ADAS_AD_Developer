//
// Created by liudian on 2022/3/9.
//

#ifndef NODEDRIVING_SUBSYSTEM_LIBRARY_H
#define NODEDRIVING_SUBSYSTEM_LIBRARY_H
#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <nodes/FlowScene>
#include <nodes/DataModelRegistry>
#include "invocable.hpp"

class ModuleLibrary;

class SubsystemLibrary {
public:
    void setPath(const std::filesystem::path & path);
    bool hasSubsystem(const std::string & package, const std::string & name);
    std::filesystem::path newSubsystem(const std::string & package, const std::string & name);
    std::filesystem::path getSubsystem(const std::string & package, const std::string & name);
    std::vector<Invocable> getInvocableList();
    explicit SubsystemLibrary(ModuleLibrary *moduleLibrary);
private:
    void readScene(QtNodes::FlowScene &scene, const std::filesystem::path & path);
    void parsePorts(const QtNodes::FlowScene &scene, Invocable & invocable);
    void onImportCompleted();
    std::filesystem::path subsystemPath(const std::string & package, const std::string & name);
    ModuleLibrary *moduleLibrary_;
    std::filesystem::path path_;
    std::shared_ptr<QtNodes::DataModelRegistry> dataModelRegistry_;





};


#endif //NODEDRIVING_SUBSYSTEM_LIBRARY_H
