#ifndef AICCFLOWSCENE_H
#define AICCFLOWSCENE_H

#include <nodes/FlowView>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <QDebug>
#include <nodeparser/module_library.hpp>


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
        auto type = this->registry().create(name);
        if(type){
            QtNodes::Node &node = this->createNode(std::move(type));
            node.nodeGraphicsObject().setPos(posView);
            this->nodePlaced(node);
        }
    }




Q_SIGNALS:
    void getNodeDataModel(NodeDataModel *nodeDataModel);

};

#endif // AICCFLOWSCENE_H
