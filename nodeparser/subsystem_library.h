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
    void setSystemPath(const std::filesystem::path & path);
    bool hasSubsystem(const std::string & package, const std::string & name) const;
    bool isSystemSubsystem(const std::string & package, const std::string & name) const;
    std::filesystem::path newSubsystem(const std::string & package, const std::string & name);
    std::filesystem::path getSubsystem(const std::string & package, const std::string & name) const;
    std::vector<Invocable> getInvocableList() const;
private:
    static boost::json::object readScene(const std::filesystem::path & path) ;
    static void parsePorts(boost::json::object &scene, Invocable & invocable) ;
    std::filesystem::path subsystemPath(const std::string & package, const std::string & name) const;
    std::filesystem::path systemSubsystemPath(const std::string & package, const std::string & name) const;
    static Invocable invocableFromPath(const std::filesystem::path & path) ;
    std::filesystem::path path_;
    std::filesystem::path systemPath_;





};


#endif //NODEDRIVING_SUBSYSTEM_LIBRARY_H
