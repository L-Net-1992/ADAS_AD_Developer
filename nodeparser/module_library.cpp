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
namespace fs = std::filesystem;

void ModuleLibrary::importFiles(const QStringList &files) {
//    std::list<Invocable> parse_result;
    InvocableParser parser;
    _packageLibrary.clear();
    _parseResult.clear();
//    for(const auto & file: files) {
    for(int i=0;i<files.count();i++){
        const auto &file = files[i];
        fs::path p = file.toStdString();
        _packageLibrary.load_prefix_paths_from_config_file(p);
        std::string error_message;
        if(!parser.parse(_packageLibrary, _parseResult, error_message)) {
            emit errorOccured(QString("解析文件 '%1' 时发生错误: %2").arg(file, QString::fromStdString(error_message)));
        } else {
            qDebug() << "result size: " << _parseResult.size();
            setInvocables(_parseResult);
        }
        emit fileParserCompleted(files.count(),i);
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

void ModuleLibrary::newSubsystem(QWidget *parent) {
    NewSubsystemDialog dialog(parent);
    NewSubsystemDialog::SubsystemNameType subsystem_name;
    for(;;) {
        if(!dialog.exec())
            return;
        subsystem_name = dialog.getSubsystemName();
        if(subsystem_name.first.empty() || subsystem_name.second.empty()) {
            QMessageBox::critical(parent, "错误", "包名或子系统名不能为空");
            continue;
        }
        if(_subsystemLibrary.hasSubsystem(subsystem_name.first, subsystem_name.second)) {
            QMessageBox::critical(parent, "错误", "子系统已经存在");
            continue;
        }
        break;
    }
    _subsystemLibrary.newSubsystem(subsystem_name.first, subsystem_name.second);
    emit importCompleted();
    openSubsystem(parent, subsystem_name.first, subsystem_name.second);

}

void ModuleLibrary::openSubsystem(QWidget *parent, const std::string &package, const std::string &name) {
    auto * subsystemWindow = new SubsystemWindow(this, _subsystemLibrary.getSubsystem(package, name) ,parent);
    subsystemWindow->show();

}

void ModuleLibrary::setSubsystemPath(const std::filesystem::path &path) {
    _subsystemLibrary.setPath(path);
    emit importCompleted();

}

