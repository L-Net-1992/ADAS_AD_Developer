//
// Created by liudian on 2022/3/8.
//

// You may need to build the project (run Qt uic code generator) to get "ui_subsystem_window.h" resolved

#include "subsystem_window.h"
#include "ui_subsystem_window.h"
#include <iostream>
#include <fstream>
#include <QByteArray>
#include "ADAS_AD_Backend/models.hpp"
SubsystemWindow::SubsystemWindow(ModuleLibrary *module_library, const std::filesystem::path & subsystem_path ,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SubsystemWindow),
    module_library_(module_library),
    parent_(parent),
    subsystem_path_(subsystem_path)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
//    scene_.setParent(parent_);
    scene_.setWparent(this);
    connect(&scene_, &QtNodes::FlowScene::nodeDoubleClicked, this, &SubsystemWindow::nodeDoubleClicked);
    connect(&scene_, &QtNodes::FlowScene::nodeCreated, this, &SubsystemWindow::generateVarName);
//    connect(&scene_,&AICCFlowScene::nodeContextMenu,this,&SubsystemWindow::updateVarName);
    connect(ui->actionSave, &QAction::triggered, this, &SubsystemWindow::save);
//    connect(this,&QMainWindow::destroyed,this,[&](){});

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
    scene_.setParent(ui->flowView->parent());
    std::cout << "new SubsystemWindow" << std::endl;

    //接受view的node创建完成消息增加新的page
    connect(ui->flowView,&AICCFlowView::checkSubSystemName,this,[&](const QString &name,const QString &caption,const QPoint pos,const AICCFlowView *cview){
        QPointF posView = ui->flowView->mapToScene(pos);
        scene_.dropCreateNode(name,caption,posView);
    });
}

SubsystemWindow::~SubsystemWindow() {
    std::cout << "delete SubsystemWindow" << std::endl;
    delete ui;
}

/**
 * @brief setBusinessData       创建完新子窗口后,可使用该函数为其他的业务数据赋值
 * @param subsystemDataModel
 */
void SubsystemWindow::setBusinessData(const std::map<std::string,std::string> & subsystemDataModel){
    subsystem_data_model_ = subsystemDataModel;
}


/**
 * @brief SubsystemWindow::sceneLoadFromMemoryCompletedAction   主场景加载完成后初始化连接node的创建、删除信号。该函数暂未使用，在保存操作时发送更新结构树操作
 * @param isCompleted
 */
void SubsystemWindow::sceneLoadFromMemoryCompletedAction(bool isCompleted){
    //    qDebug() << "subsystem window scene load from memory completed action";
    if(isCompleted){

        connect(&scene_,&QtNodes::FlowScene::nodeCreated,this,[&](QtNodes::Node &node){
            const auto *model = static_cast<InvocableDataModel*>(node.nodeDataModel());
            const auto & invocable = model->invocable();
            if(invocable.getType()==Invocable::Subsystem)
                emit subsystemCreatedOrDeleted();

        });

        //在响应节点删除后信号前先判断该节点是什么类型
        Invocable::Type invocableType = Invocable::Class;
        connect(&scene_,&QtNodes::FlowScene::nodeDeleted,this,[&](QtNodes::Node &node){
            invocableType = static_cast<InvocableDataModel*>(node.nodeDataModel())->invocable().getType();
        });

        connect(&scene_,&QtNodes::FlowScene::afterNodeDeleted,this,[&](){
            //判断当前记录的invocableType类型为Subsysem才执行刷新操作
            if(invocableType==Invocable::Subsystem)
                emit subsystemCreatedOrDeleted();
        });


    }
}

//QtNodes::FlowScene &SubsystemWindow::scene() {
AICCFlowScene &SubsystemWindow::scene() {
    return scene_;
}


void SubsystemWindow::nodeDoubleClicked(QtNodes::Node & node) {
    auto nodeDataModel = static_cast<InvocableDataModel*>(node.nodeDataModel());
    const auto invocable = nodeDataModel->invocable();
    if(invocable.getType() == Invocable::Subsystem){
//        module_library_->openSubsystem(parent_, invocable.getPackage(), invocable.getSubsystemName());
        auto subsystemWindow = new SubsystemWindow(module_library_, module_library_->subsystemLibrary().getSubsystem(invocable.getPackage(),invocable.getSubsystemName()), this);
        //当子系统有node创建或删除时，将信号继续传送到外部
        connect(subsystemWindow,&SubsystemWindow::subsystemCreatedOrDeleted,this,[&]{
            emit subsystemCreatedOrDeleted();
        });
        subsystemWindow->show();


    }
}

void SubsystemWindow::updateRegistry() {
    scene_.setRegistry(module_library_->test2());

}

void SubsystemWindow::save() {
    //数据保存到flow文件中成功
    auto buffer = scene_.saveToMemory();
    std::ofstream file(subsystem_path_, std::ios::binary);
    file.write(buffer.data(), buffer.size());
    file.close();

    if(subsystem_data_model_.size()>0){
        std::string category_path = subsystem_data_model_.at("category");
        int pid = getParentidByPath(category_path,0);
        const std::string name = subsystem_data_model_.at("name");
        const std::string package = subsystem_data_model_.at("package");
        const std::string caption = subsystem_data_model_.at("caption");
        const std::string class_name = package+"::"+name;

        if(addSubsystem2DB(pid,package,class_name,caption)){
            qInfo() << QString::fromStdString(class_name) << "业务信息写入成功";
        }else{
            qInfo() << QString::fromStdString(class_name) << "业务信息写入失败";
        }
    }


    emit module_library_->importCompleted();
    emit subsystemCreatedOrDeleted();

}


void SubsystemWindow::closeEvent(QCloseEvent *event){
//    QMessageBox::StandardButton result = QMessageBox::warning(this,QStringLiteral("退出"),QStringLiteral("请确认组合模块已保存再退出"),QMessageBox::Save|QMessageBox::Ok|QMessageBox::Cancel);
//    switch(result){
//        case QMessageBox::Ok:
//        event->accept();
//        break;
//    case QMessageBox::Cancel:
//        event->ignore();
//        break;
//    case QMessageBox::Save:
//        save();
//        event->ignore();
//        break;
//    }
}

//void SubsystemWindow::destroyed(){
//    if(0 == QMessageBox::warning(this,QStringLiteral("退出"),QStringLiteral("请确认组合模块已保存再退出"),QStringLiteral("确定"),QStringLiteral("取消"),QStringLiteral("保存"))){
//        e->accept();
//    }
//}


void SubsystemWindow::generateVarName(QtNodes::Node &node) {
//    ModuleLibrary::generateVarNameIfEmpty(scene_, node);
    scene_.generateVarNameIfEmpty(scene_.allNodes(),node);
}

//void SubsystemWindow::updateVarName(QtNodes::Node &node, const QPointF & pos) {

//    AICCFlowScene::updateVarName(scene_.allNodes(),node,this);



//    auto *nodeDataModel = static_cast<InvocableDataModel *>(node.nodeDataModel());
//    VarNameDialog dialog(parent);
//    dialog.setVarName(nodeDataModel->varName());
//    for (;;) {
//        if (!dialog.exec())
//            return;
//        auto var_name = dialog.varName();
//        if (var_name.isEmpty()) {
//            QMessageBox::critical(parent, "错误", "名称不能为空");
//            continue;
//        }
//        if (varNameExists(var_name)) {
//            QMessageBox::critical(parent, "错误", "名称已经被使用");
//            continue;
//        }
//        nodeDataModel->setVarName(var_name);
//        return;
//    }

//}

