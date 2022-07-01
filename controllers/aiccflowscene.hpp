#ifndef AICCFLOWSCENE_H
#define AICCFLOWSCENE_H

#include <nodes/FlowView>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <QDebug>
#include <nodeparser/module_library.hpp>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMimeData>


using QtNodes::FlowView;
using QtNodes::FlowScene;
using QtNodes::Node;
using QtNodes::NodeGraphicsObject;
using QtNodes::NodeDataModel;

class AICCFlowScene : public FlowScene
{
    Q_OBJECT
public:
    AICCFlowScene(QObject * parent = Q_NULLPTR):FlowScene(parent)
    {

        connect(this,&FlowScene::selectionChanged,this,[&]()
        {
            auto selectedCount = this->selectedNodes().size();
            if(selectedCount==1){
                emit getNodeDataModel(this->selectedNodes()[0]->nodeDataModel());
            }
            else{
                emit getNodeDataModel(Q_NULLPTR);
            }
        });

        this->addText("(0,0)");
        this->addEllipse(0,0,1,1);

        connect(this,&AICCFlowScene::nodeContextMenu,this,&AICCFlowScene::nodeContextMenuAction);



        //        this->addEllipse(72,1,1,1);
        //        this->addEllipse(72,94,1,1);
        //当场景文件加载完成后，连接节点创建信号与节点删除信号
        //        connect(this,&FlowScene::sceneLoadFromMemoryCompleted,this,[&](bool isCompleted){
        //            if(isCompleted){
        //                initConnect();
        //            }
        //        });
    }
    ~AICCFlowScene(){}

    ///在FlowScene中创建node
    void dropCreateNode(const QString  &name,const QString &caption,const QPointF posView)
    {
//        std::cout << "aicc flow scene:" << name.toStdString() << " " << caption.toStdString() << "   categoryies size:" << this->registry().categories().size();
        try{
            auto type = this->registry().create(name);

            if(type){
                QtNodes::Node &node = this->createNode(std::move(type));
                node.nodeGraphicsObject().setPos(posView);
                this->nodePlaced(node);
            }
        }catch(const std::exception &e){
            std::cout << "DataModelRegistry:" << e.what();
        }
    }

private:


    void nodeContextMenuAction (Node &node,const QPointF &pos){
        QClipboard *clipboard = QApplication::clipboard();

        QMenu *pop_menu = new QMenu();

        QAction *act_copy = new QAction(pop_menu);
        act_copy->setText(QStringLiteral("复制"));
        act_copy->setIcon(QIcon(":/res/editor-copy.png"));
        pop_menu->addAction(act_copy);

        QAction *act_paste = new QAction(pop_menu);
        act_paste->setText(QStringLiteral("粘贴"));
        act_paste->setIcon(QIcon(":/res/editor-paste.png"));
        pop_menu->addAction(act_paste);
        if(clipboard->property("nodes").toStringList().size()==0)
            act_paste->setEnabled(false);

        pop_menu->addSeparator();

        QAction *act_rename = new QAction(pop_menu);
        act_rename->setText(QStringLiteral("重命名"));
        act_rename->setIcon(QIcon(":/res/rename.png"));
        pop_menu->addAction(act_rename);

        //设置菜单功能
        connect(act_copy,&QAction::triggered,this,[&](){
            std::vector<Node*> nodes =  this->selectedNodes();
            QStringList sl_nodes;
            for(Node* node:nodes){
                auto *ndm = static_cast<InvocableDataModel *>(node->nodeDataModel());
                sl_nodes.append(ndm->name());
            }
            clipboard->setProperty("nodes",sl_nodes);
        });

        //粘贴
        connect(act_paste,&QAction::triggered,this,[&](){
            QClipboard *clipboard = QApplication::clipboard();
            QStringList sl = clipboard->property("nodes").toStringList();
            for(int i=0;i<sl.size();i++){
                QString nn = sl.at(i);
                QPointF p(pos.x()+(i*10),pos.y()+(i*10));
                dropCreateNode(nn,nn,p);
            }
        });

        //重命名
        connect(act_rename,&QAction::triggered,this,[&](){
           emit nodeRename(node);
        });
        pop_menu->exec(QCursor::pos());

//        //回收action资源
        foreach(QAction *pAction,pop_menu->actions()) delete pAction;
    }



Q_SIGNALS:
    void getNodeDataModel(NodeDataModel *nodeDataModel);
    void nodeRename(Node &node);

private:
    QSharedPointer<QAction> _pop_menu;
    QSharedPointer<QAction> _act_copy;
    QSharedPointer<QAction> _act_paste;
    QSharedPointer<QAction> _act_rename;
};

#endif // AICCFLOWSCENE_H
