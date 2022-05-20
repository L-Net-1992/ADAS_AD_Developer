//
// Created by 刘典 on 2021/9/12.
//

#include "module_library.hpp"
#include "invocable_parser.hpp"
#include "new_subsystem_dialog.h"
#include "subsystem_window.h"
#include <algorithm>
#include <QtDebug>
#include <QMessageBox>
#include "var_name_dialog.h"

namespace fs = std::filesystem;

void ModuleLibrary::importFiles(const QStringList &files) {
//    std::list<Invocable> parse_result;
    InvocableParser parser;
    _packageLibrary.clear();
    _parseResult.clear();
//    for(const auto & file: files) {
    for (int i = 0; i < files.count(); i++) {
        const auto &file = files[i];
        fs::path p = file.toStdString();
        _packageLibrary.load_prefix_paths_from_config_file(p);
        std::string error_message;
        if (!parser.parse(_packageLibrary, _parseResult, error_message)) {
            emit errorOccured(QString("解析文件 '%1' 时发生错误: %2").arg(file, QString::fromStdString(error_message)));
        } else {
//            qDebug() << "result size: " << _parseResult.size();
            setInvocables(_parseResult);
        }
        emit fileParserCompleted(files.count(), i);
    }
}


int ModuleLibrary::rowCount(const QModelIndex &parent) const {
    return _invocableList.size();
}

QVariant ModuleLibrary::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    if (index.row() >= _invocableList.size())
        return {};

    if (role == Qt::DisplayRole)
        return QString::fromStdString(_invocableList.at(index.row()).getName());
    else
        return {};
}

void ModuleLibrary::setInvocables(const std::list<Invocable> &list) {
    beginResetModel();
    _invocableList.clear();
    std::copy(list.begin(), list.end(), std::back_inserter(_invocableList));
    endResetModel();
    emit importCompleted();

}

void ModuleLibrary::setCurrentUseCategoryFullPath(const QStringList &newCurrentUseCategoryFullPath)
{
    _currentUseCategoryFullPath = newCurrentUseCategoryFullPath;
}


void ModuleLibrary::newSubsystem(QWidget *parent) {
    NewSubsystemDialog dialog(parent);
    dialog.setCategoryComboBox(_currentUseCategoryFullPath);
    NewSubsystemDialog::SubsystemNameType subsystem_name;
    for (;;) {
        if (!dialog.exec())
            return;
        subsystem_name = dialog.getSubsystemName();
        if (subsystem_name.first.empty() || subsystem_name.second.empty()) {
            QMessageBox::critical(parent, "错误", "包名或子系统名不能为空");
            continue;
        }
        if (_subsystemLibrary.hasSubsystem(subsystem_name.first, subsystem_name.second)) {
            QMessageBox::critical(parent, "错误", "子系统已经存在");
            continue;
        }
        break;
    }
    _subsystemLibrary.newSubsystem(subsystem_name.first, subsystem_name.second);
    emit importCompleted();

//    openSubsystem(parent, subsystem_name.first, subsystem_name.second);
    openSubsystem(parent,dialog.getSubsystemDataModel());

}

void ModuleLibrary::openSubsystem(QWidget *parent, const std::string &package, const std::string &name) {
    auto subsystemWindow = new SubsystemWindow(this, _subsystemLibrary.getSubsystem(package, name), parent);
    //当子系统有node创建或删除时，将信号继续传送到外部
    connect(subsystemWindow,&SubsystemWindow::subsystemCreatedOrDeleted,this,[&]{
        emit subsystemCreatedOrDeleted();
    });
    subsystemWindow->show();

}

void ModuleLibrary::openSubsystem(QWidget *parent, const std::map<std::string,std::string> &subsystemDataModel){
    const std::string package = subsystemDataModel.at("package");
    const std::string name = subsystemDataModel.at("name");
    auto subsystemWindow = new SubsystemWindow(this, _subsystemLibrary.getSubsystem(package, name), parent);
    //当子系统有node创建或删除时，将信号继续传送到外部
    connect(subsystemWindow,&SubsystemWindow::subsystemCreatedOrDeleted,this,[&]{
        emit subsystemCreatedOrDeleted();
    });
    subsystemWindow->show();
    subsystemWindow->setBusinessData(subsystemDataModel);
}

void ModuleLibrary::setSubsystemPath(const std::filesystem::path &path) {
    _subsystemLibrary.setPath(path);
    emit importCompleted();

}

void ModuleLibrary::updateVarName(QtNodes::FlowScene &scene, QtNodes::Node &node, QWidget *parent) {
    auto *nodeDataModel = static_cast<InvocableDataModel *>(node.nodeDataModel());
    VarNameDialog dialog(parent);
    dialog.setVarName(nodeDataModel->varName());
    for (;;) {
        if (!dialog.exec())
            return;
        auto var_name = dialog.varName();
        if (var_name.isEmpty()) {
            QMessageBox::critical(parent, "错误", "名称不能为空");
            continue;
        }
        if (varNameExists(scene, var_name)) {
            QMessageBox::critical(parent, "错误", "名称已经被使用");
            continue;
        }
        nodeDataModel->setVarName(var_name);
        return;
    }

}

bool ModuleLibrary::varNameExists(QtNodes::FlowScene &scene, const QString &var_name) {
    for (const auto *node: scene.allNodes()) {
        const auto *nodeDataModel = static_cast<const InvocableDataModel *>(node->nodeDataModel());
        if (nodeDataModel->varName() == var_name)
            return true;

    }
    return false;
}

QString ModuleLibrary::generateVarName(QtNodes::FlowScene &scene) {
    QString fmt("n%1");
    for (int i = 1;; i++) {
        QString var_name = fmt.arg(i);
        if (!varNameExists(scene, var_name))
            return var_name;

    }
}

void ModuleLibrary::generateVarNameIfEmpty(QtNodes::FlowScene &scene, QtNodes::Node &node) {
    auto *nodeDataModel = static_cast<InvocableDataModel *>(node.nodeDataModel());
    if (nodeDataModel->varName().isEmpty())
        nodeDataModel->setVarName(generateVarName(scene));
}

void ModuleLibrary::addPackage(const std::filesystem::path &path) {
    _packageLibrary.add_prefix_path(path);
    InvocableParser parser;
    std::string error_message;
    _parseResult.clear();
    if (!parser.parse(_packageLibrary, _parseResult, error_message)) {
        emit errorOccured(QString("解析文件 '%1' 时发生错误: %2").arg(QString::fromStdString(path.string()),
                                                             QString::fromStdString(error_message)));
    } else {
//        qDebug() << "result size: " << _parseResult.size();
        setInvocables(_parseResult);
    }
    emit fileParserCompleted(1, 0);
}

