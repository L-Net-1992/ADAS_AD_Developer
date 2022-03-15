//
// Created by liudian on 2022/3/8.
//

#ifndef NODEDRIVING_SUBSYSTEM_WINDOW_H
#define NODEDRIVING_SUBSYSTEM_WINDOW_H

#include <QMainWindow>
#include "ui_subsystem_window.h"
#include <nodes/FlowScene>
#include <nodes/Node>
#include "module_library.hpp"
#include <filesystem>

QT_BEGIN_NAMESPACE
namespace Ui { class SubsystemWindow; }
QT_END_NAMESPACE

class SubsystemWindow : public QMainWindow {
Q_OBJECT

public:
    explicit SubsystemWindow(ModuleLibrary *module_library, const std::filesystem::path & subsystem_path ,QWidget *parent = nullptr);

    ~SubsystemWindow() override;
    QtNodes::FlowScene & scene();


private:
    Ui::SubsystemWindow *ui;
    QtNodes::FlowScene scene_;
    ModuleLibrary *module_library_;
    QWidget *parent_;
    std::filesystem::path subsystem_path_;
    void save();

private Q_SLOTS:
    void nodeDoubleClicked(QtNodes::Node & node);
    void updateRegistry();
    void generateVarName(QtNodes::Node & node);
    void updateVarName(QtNodes::Node & node, const QPointF & pos);
};


#endif //NODEDRIVING_SUBSYSTEM_WINDOW_H
