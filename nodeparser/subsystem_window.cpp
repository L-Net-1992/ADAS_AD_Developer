//
// Created by liudian on 2022/3/8.
//

// You may need to build the project (run Qt uic code generator) to get "ui_subsystem_window.h" resolved

#include "subsystem_window.h"
#include "ui_subsystem_window.h"
#include <iostream>
#include <fstream>
#include <QByteArray>
#include "models.hpp"
SubsystemWindow::SubsystemWindow(ModuleLibrary *module_library, const std::filesystem::path & subsystem_path ,QWidget *parent) :
        QMainWindow(parent), ui(new Ui::SubsystemWindow), module_library_(module_library), parent_(parent), subsystem_path_(subsystem_path) {
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    connect(&scene_, &QtNodes::FlowScene::nodeDoubleClicked, this, &SubsystemWindow::nodeDoubleClicked);
    connect(&scene_, &QtNodes::FlowScene::nodeCreated, this, &SubsystemWindow::generateVarName);
    connect(&scene_, &QtNodes::FlowScene::nodeContextMenu, this, &SubsystemWindow::updateVarName);
    connect(ui->actionSave, &QAction::triggered, this, &SubsystemWindow::save);
    scene_.setRegistry(module_library->test2());
    int file_size = static_cast<int>(std::filesystem::file_size(subsystem_path));
    QByteArray buffer(file_size, 0);
    std::ifstream file(subsystem_path, std::ios::binary);
    file.read(buffer.data(), buffer.size());
    scene_.loadFromMemory(buffer);
    QString package = QString::fromStdString(subsystem_path.parent_path().filename().string());
    QString name = QString::fromStdString(subsystem_path.filename().stem().string());
    setWindowTitle(QString("子系统 - %1::%2").arg(package, name));
    connect(module_library, &ModuleLibrary::importCompleted, this, &SubsystemWindow::updateRegistry);
    ui->flowView->setScene(&scene_);
    std::cout << "new SubsystemWindow" << std::endl;
}

SubsystemWindow::~SubsystemWindow() {
    std::cout << "delete SubsystemWindow" << std::endl;
    delete ui;
}

QtNodes::FlowScene &SubsystemWindow::scene() {
    return scene_;
}

void SubsystemWindow::nodeDoubleClicked(QtNodes::Node & node) {
    auto * nodeDataModel = static_cast<InvocableDataModel*>(node.nodeDataModel());
    const auto & invocable = nodeDataModel->invocable();
    if(invocable.getType() == Invocable::Subsystem)
        module_library_->openSubsystem(parent_, invocable.getPackage(), invocable.getSubsystemName());

}

void SubsystemWindow::updateRegistry() {
    scene_.setRegistry(module_library_->test2());

}

void SubsystemWindow::save() {
    auto buffer = scene_.saveToMemory();
    std::ofstream file(subsystem_path_, std::ios::binary);
    file.write(buffer.data(), buffer.size());
    file.close();
    emit module_library_->importCompleted();

}

void SubsystemWindow::generateVarName(QtNodes::Node &node) {
    ModuleLibrary::generateVarNameIfEmpty(scene_, node);

}

void SubsystemWindow::updateVarName(QtNodes::Node &node, const QPointF & pos) {
    ModuleLibrary::updateVarName(scene_, node, this);

}

