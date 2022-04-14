#ifndef AICCSTRUCTTREEWIDGET_H
#define AICCSTRUCTTREEWIDGET_H

#include <QTreeWidget>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDebug>
#include <QSplitter>
#include <QMimeData>
#include <QDrag>
#include <QMenu>
#include <QTime>
#include <QVBoxLayout>
#include <conf/Config.hpp>
#include <QApplication>
#include "utils.h"
#include <QStandardItem>
#include <QTreeWidgetItem>
#include <nodes/FlowScene>
#include <nodes/Node>
#include "nodeparser/invocable.hpp"
#include "nodeparser/models.hpp"
#include "nodeparser/module_library.hpp"

using QtNodes::FlowScene;
using QtNodes::Node;

class AICCStructTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit AICCStructTreeWidget(QWidget *parent=nullptr){
        //设置默认双击时不能展开收缩
        this->setExpandsOnDoubleClick(false);
        //双击打开子系统窗口
        connect(this,&AICCStructTreeWidget::itemDoubleClicked,this,[&](QTreeWidgetItem *item,int column){
            emit treeNodeDoubleClicked(item);
        });

        icon.addPixmap(QPixmap(":/res/Open.png"));
    }

    ~AICCStructTreeWidget(){}

    ///初始化左侧功能树数据,主界面使用
    void fillInitLeftTreeData(ModuleLibrary *module_library,SubsystemLibrary *subsystem_library, const QString &pname,const FlowScene *scene){

        //        this->setDragDropMode(QAbstractItemView::DragOnly);
        //        this->setDragEnabled(true);
        this->clear();

        //设置根节点
        QTreeWidgetItem *twi_root = new QTreeWidgetItem(this);
        twi_root->setText(0,pname);
        twi_root->setIcon(0,icon);

        std::vector<Node*> v_nodes = scene->allNodes();
        //在刷新左侧菜单之前，缓存DataModelRegistry对象
        dmr = module_library->test2();
        makeTreeWidgetItem(v_nodes,module_library,subsystem_library,twi_root);

        this->expandAll();
    }

Q_SIGNALS:
    void treeNodeDoubleClicked(QTreeWidgetItem *item);

private:
    ///解析所有子系统的节点
    void parseSubSystemNode(ModuleLibrary *module_library,SubsystemLibrary *subsystem_library,QTreeWidgetItem *ptwi,const Node* node) {

        qDebug() << "begin parse:" << QString::number(QTime::currentTime().msec());
        const auto *model = static_cast<InvocableDataModel*>(node->nodeDataModel());
        const auto & invocable = model->invocable();
        std::filesystem::path subsystem_path = subsystem_library->getSubsystem(invocable.getPackage(),invocable.getSubsystemName());
        qDebug() << "   ----FlowScene:" << QString::number(QTime::currentTime().msec());
        //将子系统的flow文件加载到scene中
        FlowScene scene;
        scene.setRegistry(dmr);
        scene.loadFromMemory(loadFlowFile(subsystem_path));

        //获得scene中的所有node对象
        std::vector<Node*> v_nodes = scene.allNodes();
        qDebug() << "end parse:" << QString::number(QTime::currentTime().msec());
        makeTreeWidgetItem(v_nodes,module_library,subsystem_library,ptwi);
    }

    ///生成子系统控件的节点
    void makeTreeWidgetItem(std::vector<Node*> v_nodes,ModuleLibrary *module_library,SubsystemLibrary *subsystem_library,QTreeWidgetItem *ptwi){

        for(const Node* node:v_nodes){
            const auto *model = static_cast<const InvocableDataModel*>(node->nodeDataModel());
            if(model->invocable().getType()==Invocable::Subsystem){
                QString ncaption = node->nodeDataModel()->caption();
                QTreeWidgetItem *twi = new QTreeWidgetItem(ptwi);
                twi->setText(0,ncaption);
                twi->setIcon(0,icon);
                parseSubSystemNode(module_library,subsystem_library,twi,node);
            }
        }
    }

    ///加载flow文件，以QByteArray形式返回
    QByteArray loadFlowFile(const std::filesystem::path subsystem_path){
//        if(mapFlowFileCache.contains(subsystem_path.string())){
//            qDebug() << "cache:" << QString::fromStdString(subsystem_path.string());
//            return mapFlowFileCache.value(subsystem_path.string());
//        }

        int file_size = static_cast<int>(std::filesystem::file_size(subsystem_path));
        QByteArray buffer(file_size,0);
        std::ifstream file(subsystem_path,std::ios::binary);
        file.read(buffer.data(),buffer.size());
//        mapFlowFileCache.insert(subsystem_path.string(),buffer);
        return buffer;
    }

private:
    QIcon icon;
    std::shared_ptr<QtNodes::DataModelRegistry> dmr;            //缓存DataModelregistry数据

};

#endif // AICCSTRUCTTREEWIDGET_H
