#ifndef AICCFLOWSCENE_H
#define AICCFLOWSCENE_H

#include <nodes/FlowView>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <QDebug>
//#include <module_library.hpp>
#include "ADAS_AD_Backend/models.hpp"
#include "dialogs/var_name_dialog.h"
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMimeData>
#include <QShortcut>
#include <QKeySequence>
#include <QApplication>

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

        connect(this,&AICCFlowScene::nodeContextMenu,this,&AICCFlowScene::nodeContextMenuAction);

        this->addText("(0,0)");
        this->addEllipse(0,0,1,1);
    }
    ~AICCFlowScene(){}

    ///在FlowScene中创建node
    void dropCreateNode(const QString  &name,const QString &caption,const QPointF posView)
    {
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

public:
    void actionCopy(){
        QClipboard *clipboard = QApplication::clipboard();
        std::vector<Node*> nodes =  this->selectedNodes();
        QStringList sl_nodes;
        for(Node* node:nodes){
            auto *ndm = static_cast<InvocableDataModel *>(node->nodeDataModel());
            sl_nodes.append(ndm->name());
        }
        clipboard->setProperty("nodes",sl_nodes);
    }
    void actionPaste(){
        QClipboard *clipboard = QApplication::clipboard();
        QStringList sl = clipboard->property("nodes").toStringList();
//        QPointF pos = this->mapFromScene(QCursor::pos());
        QPointF pos = this->selectedNodes().at(0)->nodeGraphicsObject().pos();

        for(int i=0;i<sl.size();i++){
            QString nn = sl.at(i);
            QPointF p(pos.x()+((i+1)*30),pos.y()+((i+1)*30));
            this->dropCreateNode(nn,nn,p);
        }
    }

    void nodeContextMenuAction(Node &node,const QPointF &pos){
        QClipboard *clipboard = QApplication::clipboard();

        QMenu *pop_menu = new QMenu();

        QAction *act_copy = new QAction(pop_menu);
        act_copy->setText(QStringLiteral("复制"));
        act_copy->setIcon(QIcon(":/res/editor-copy.png"));
        act_copy->setShortcut(QKeySequence::Copy);
        pop_menu->addAction(act_copy);

        QAction *act_paste = new QAction(pop_menu);
        act_paste->setText(QStringLiteral("粘贴"));
        act_paste->setIcon(QIcon(":/res/editor-paste.png"));
        act_paste->setShortcut(QKeySequence::Paste);
        pop_menu->addAction(act_paste);
        if(clipboard->property("nodes").toStringList().size()==0)
            act_paste->setEnabled(false);

        pop_menu->addSeparator();

        QAction *act_rename = new QAction(pop_menu);
        act_rename->setText(QStringLiteral("重命名"));
        act_rename->setIcon(QIcon(":/res/rename.png"));
        act_rename->setShortcutContext(Qt::ApplicationShortcut);
        pop_menu->addAction(act_rename);

        //复制
        connect(act_copy,&QAction::triggered,this,&AICCFlowScene::actionCopy);

        //粘贴
        connect(act_paste,&QAction::triggered,this,&AICCFlowScene::actionPaste);

        //重命名
        connect(act_rename,&QAction::triggered,this,[&](){
            this->updateVarName(allNodes(), node, _wparent);
        });



        pop_menu->exec(QCursor::pos());

        //        //回收action资源
        foreach(QAction *pAction,pop_menu->actions()) delete pAction;
    }

    void updateVarName(const std::vector<Node*> nodes,Node &node,QWidget *parent){

        auto *nodeDataModel = static_cast<InvocableDataModel *>(node.nodeDataModel());
        VarNameDialog dialog(parent);
        dialog.setVarName(nodeDataModel->varName());
        for (;;) {
            if (!dialog.exec())
                return;
            auto var_name = dialog.varName();
//            dialog.move(100,100);
            if (var_name.isEmpty()) {
                QMessageBox::critical(parent, "错误", "名称不能为空");
                continue;
            }
            if (varNameExists(nodes,var_name)) {
                QMessageBox::critical(parent, "错误", "名称已经被使用");
                continue;
            }
            nodeDataModel->setVarName(var_name);
            return;
        }
    }


    bool varNameExists(const std::vector<Node*> nodes,const QString &var_name){
        for (const auto *node: nodes) {
            const auto *nodeDataModel = static_cast<const InvocableDataModel *>(node->nodeDataModel());
            if (nodeDataModel->varName() == var_name)
                return true;
        }
        return false;
    }

    QString generateVarName(const std::vector<Node*> nodes) {
        QString fmt("n%1");
        for (int i = 1;; i++) {
            QString var_name = fmt.arg(i);
            if (!varNameExists(nodes, var_name))
                return var_name;
        }
    }

    void generateVarNameIfEmpty(const std::vector<Node*> nodes,QtNodes::Node &node) {
        auto *nodeDataModel = static_cast<InvocableDataModel *>(node.nodeDataModel());
        if (nodeDataModel->varName().isEmpty())
            nodeDataModel->setVarName(generateVarName(nodes));
    }

    void setWparent(QWidget *newWparent){_wparent = newWparent;};

Q_SIGNALS:
    void getNodeDataModel(NodeDataModel *nodeDataModel);


private:
    QMenu *_pop_menu;
    QSharedPointer<QAction> _act_copy;
    QSharedPointer<QAction> _act_paste;
    QAction * _act_rename;
    QWidget * _wparent;
};

#endif // AICCFLOWSCENE_H
