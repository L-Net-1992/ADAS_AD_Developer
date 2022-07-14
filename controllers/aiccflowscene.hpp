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
//    void updateVarName(FlowScene &scene,Node &node,QWidget *parent){
    void updateVarName(Node &node,QWidget *parent){
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
                if (varNameExists(*this, var_name)) {
                    QMessageBox::critical(parent, "错误", "名称已经被使用");
                    continue;
                }
                nodeDataModel->setVarName(var_name);
                return;
            }
    }

    bool varNameExists(AICCFlowScene &scene,const QString &var_name){
            for (const auto *node: scene.allNodes()) {
                const auto *nodeDataModel = static_cast<const InvocableDataModel *>(node->nodeDataModel());
                if (nodeDataModel->varName() == var_name)
                    return true;
            }
            return false;
    }

    QString generateVarName() {
        QString fmt("n%1");
        for (int i = 1;; i++) {
            QString var_name = fmt.arg(i);
            if (!varNameExists(*this, var_name))
                return var_name;
        }
    }

    void generateVarNameIfEmpty(QtNodes::Node &node) {
        auto *nodeDataModel = static_cast<InvocableDataModel *>(node.nodeDataModel());
        if (nodeDataModel->varName().isEmpty())
            nodeDataModel->setVarName(generateVarName());
    }

Q_SIGNALS:
    void getNodeDataModel(NodeDataModel *nodeDataModel);


private:
    QMenu *_pop_menu;
    QSharedPointer<QAction> _act_copy;
    QSharedPointer<QAction> _act_paste;
    QAction * _act_rename;
};

#endif // AICCFLOWSCENE_H
