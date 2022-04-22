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
        //在刷新左侧菜单之前，第一次从test2中获得所有的DataModelRegistry数据，后面每次遍历的时候使用缓存即可
        dmr = module_library->test2();
        makeTreeWidgetItem(v_nodes,module_library,subsystem_library,twi_root);

        this->expandAll();
    }

    ///过滤出制定字符串的节点
    void filterTreeWidgetItem(const QString &text){

        QVector<QTreeWidgetItem*> v_twi;
        v_twi = recursionTreeWidgetItem(text,this->topLevelItem(0),v_twi);

        for(QTreeWidgetItem *item :v_twi){
            item->setHidden(false);
        }
    }

Q_SIGNALS:
    void treeNodeDoubleClicked(QTreeWidgetItem *item);

private:
    ///遍历所有的节点，假如节点名称与字符串参数不匹配，该节点隐藏
    QVector<QTreeWidgetItem*> recursionTreeWidgetItem(const QString &text ,const QTreeWidgetItem *pitem,QVector<QTreeWidgetItem*> v_twi){
        int count = pitem->childCount();
        for(int i=0;i < count;++i){
            auto child = pitem->child(i);
            auto itemName = child->text(0);
            const bool match = (itemName.contains(text,Qt::CaseInsensitive));
            if(match){
                v_twi = recursionParentTreeWidgetItem(false,child,v_twi);
                v_twi.append(child);
                child->setHidden(true);
//                for(QTreeWidgetItem *i:v_twi)
//                    i->setHidden(false);
            }else{}
//                child->setHidden(true);
        }
        return v_twi;
    }
    ///遍历当前节点所有父级节点，设置这些节点的隐藏值
    QVector<QTreeWidgetItem*> recursionParentTreeWidgetItem(bool hidden,QTreeWidgetItem *item,QVector<QTreeWidgetItem*> v){
//        item->setHidden(hidden);
        QTreeWidgetItem *pitem = item->parent();
        if(pitem!=Q_NULLPTR){
            v.append(pitem);
//            pitem->setHidden(hidden);
//            qDebug() << "pitem:" << hidden;

            return recursionParentTreeWidgetItem(hidden,pitem,v);
        }
        return v;
    }

    ///解析所有子系统的节点
    void parseSubSystemNode(ModuleLibrary *module_library,SubsystemLibrary *subsystem_library,QTreeWidgetItem *ptwi,const Node* node) {

        const auto *model = static_cast<InvocableDataModel*>(node->nodeDataModel());
        const auto & invocable = model->invocable();
        std::filesystem::path subsystem_path = subsystem_library->getSubsystem(invocable.getPackage(),invocable.getSubsystemName());
        //将子系统的flow文件加载到scene中
        FlowScene scene;
        scene.setRegistry(dmr);
        scene.loadFromMemory(loadFlowFile(subsystem_path));

        //获得scene中的所有node对象
        std::vector<Node*> v_nodes = scene.allNodes();
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

        int file_size = static_cast<int>(std::filesystem::file_size(subsystem_path));
        QByteArray buffer(file_size,0);
        std::ifstream file(subsystem_path,std::ios::binary);
        file.read(buffer.data(),buffer.size());
        return buffer;
    }

private:
    QIcon icon;
    std::shared_ptr<QtNodes::DataModelRegistry> dmr;            //缓存DataModelregistry数据

};

#endif // AICCSTRUCTTREEWIDGET_H
