//
// Created by liudian on 2022/3/9.
//

#ifndef NODEDRIVING_SUBSYSTEM_LIBRARY_H
#define NODEDRIVING_SUBSYSTEM_LIBRARY_H
#include <filesystem>
#include <string>
#include <vector>
#include "invocable.hpp"
#include <boost/json.hpp>


class SubsystemLibrary {
public:
    void setPath(const std::filesystem::path & path);
    bool hasSubsystem(const std::string & package, const std::string & name) const;
    std::filesystem::path newSubsystem(const std::string & package, const std::string & name);
    std::filesystem::path getSubsystem(const std::string & package, const std::string & name) const;
    std::vector<Invocable> getInvocableList() const;
private:
    boost::json::object readScene(const std::filesystem::path & path) const;
    void parsePorts(boost::json::object &scene, Invocable & invocable) const;
    std::filesystem::path subsystemPath(const std::string & package, const std::string & name) const;
    std::filesystem::path path_;





};


#endif //NODEDRIVING_SUBSYSTEM_LIBRARY_H
