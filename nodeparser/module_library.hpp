//
// Created by 刘典 on 2021/9/12.
//

#ifndef NODEDRIVING_MODULE_LIBRARY_HPP
#define NODEDRIVING_MODULE_LIBRARY_HPP

#include <QStringList>
#include <filesystem>
#include <list>
#include <vector>
#include <QAbstractListModel>
#include <nodes/NodeData>
#include <nodes/FlowScene>
#include <nodes/FlowView>
#include <nodes/DataModelRegistry>
#include <nodes/NodeStyle>
#include <nodes/FlowViewStyle>
#include <nodes/ConnectionStyle>
#include "invocable.hpp"
#include "models.hpp"
#include <iostream>
#include <QWidget>
#include <QDebug>
#include <filesystem>
#include "package_library.h"
#include "subsystem_library.h"

class ModuleLibrary : public QAbstractListModel {
Q_OBJECT
public Q_SLOTS:

    void importFiles(const QStringList &files);

    void setSubsystemPath(const std::filesystem::path &path);

    void newSubsystem(QWidget *parent);

    void openSubsystem(QWidget *parent, const std::string &package, const std::string &name);


Q_SIGNALS:

    void errorOccured(const QString &error_message);

    void importCompleted();

    void fileParserCompleted(int count, int index);

    void subsystemCreatedOrDeleted();

private:
    std::vector<Invocable> _invocableList;
    std::list<Invocable> _parseResult;

    void setInvocables(const std::list<Invocable> &list);

    PackageLibrary _packageLibrary;
    SubsystemLibrary _subsystemLibrary;


public:
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    const PackageLibrary &packageLibrary() const {
        return _packageLibrary;
    }
    const SubsystemLibrary &subsystemLibrary() const {
        return _subsystemLibrary;
    }
    void addPackage(const std::filesystem::path & path);

    std::shared_ptr<QtNodes::DataModelRegistry> test2NoSubsystem() const {
        auto ret = std::make_shared<QtNodes::DataModelRegistry>();
        for (const auto &inv: _invocableList) {
            auto f = [inv]() { return std::make_unique<InvocableDataModel>(inv); };
            ret->registerModel<InvocableDataModel>(f, "test");

        }
        return ret;
    }

    std::shared_ptr<QtNodes::DataModelRegistry> test2() const{
        auto ret = std::make_shared<QtNodes::DataModelRegistry>();
        for (const auto &inv: _invocableList) {
            auto f = [inv]() { return std::make_unique<InvocableDataModel>(inv); };
            ret->registerModel<InvocableDataModel>(f, "model");

        }
        if(ret->categories().empty())
            return ret;
        for (const auto &inv: _subsystemLibrary.getInvocableList()) {
            auto f = [inv]() { return std::make_unique<InvocableDataModel>(inv); };
            ret->registerModel<InvocableDataModel>(f, "model");
        }
        return ret;
    }

    std::list<Invocable> getParseResult() { return _parseResult; }
public:
    static void updateVarName(QtNodes::FlowScene & scene, QtNodes::Node & node, QWidget * parent);
    static void generateVarNameIfEmpty(QtNodes::FlowScene & scene, QtNodes::Node & node);
private:
    static QString generateVarName(QtNodes::FlowScene & scene);
    static bool varNameExists(QtNodes::FlowScene & scene, const QString & var_name);

};


#endif //NODEDRIVING_MODULE_LIBRARY_HPP
