#ifndef AICCFLOWVIEW_H
#define AICCFLOWVIEW_H

#include <nodes/FlowView>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <QDebug>
#include <QDropEvent>
#include <QStringList>
#include <QMimeData>
#include <QtCore/QObject>
#include <QMouseEvent>
#include <QMessageBox>
#include "qvariant.h"
#include "controllers/aiccflowscene.hpp"


using QtNodes::FlowView;
using QtNodes::FlowScene;
using QtNodes::Node;
using QtNodes::NodeGraphicsObject;
using QtNodes::NodeDataModel;

///自定义FlowView
class AICCFlowView : public FlowView
{
    Q_OBJECT
public:
    AICCFlowView(){}
    AICCFlowView(AICCFlowScene *scene,QWidget *parent):FlowView(scene,parent)
    {
        _scene = scene;
//        connect(_scene,&FlowScene::selectionChanged,this,[&]()
//        {
//            auto selectedCount = _scene->selectedNodes().size();
//            if(selectedCount==1){
//                emit getNodeDataModel(_scene->selectedNodes()[0]->nodeDataModel());
//            }
//            else{
//                emit getNodeDataModel(Q_NULLPTR);
//            }
//        });

    }
public:
Q_SIGNALS:
//    void getNodeDataModel(NodeDataModel *nodeDataModel);
    //该信号需要将view对象以参数方式传递过去使用,否则会有问题
    void checkSubSystemName(const QString &name,const QString &caption,const QPoint mousePos,const AICCFlowView *cview);

protected:
    void dragMoveEvent(QDragMoveEvent *e)
    {
        //        qDebug()<<"flow view drag move";
        //        e->accept();
    };
    void dropEvent(QDropEvent *e) {
        QStringList formats = e->mimeData()->formats();
//                qDebug() << "flow view drop" << formats;
        qDebug() << e->mimeData();
        if(e->mimeData()->hasFormat("Data/name"))
        {
            QByteArray itemData = e->mimeData()->data("Data/name");
            QDataStream dataStream(&itemData,QIODevice::ReadOnly);
            QString name;
            dataStream >> name;
            qDebug() << "name:"<<name;

            QByteArray itemCaptionData = e->mimeData()->data("Data/caption");
            QDataStream dataCaptionStream(&itemCaptionData,QIODevice::ReadOnly);
            QString caption;
            dataCaptionStream >> caption;
            qDebug() << "caption:" << caption;

            //如果当前节点为子系统,通知上层AICCStackedWidget判断是否有重名,是否可创建新的page
            emit checkSubSystemName(name,caption,e->pos(),this);

            //处理消息传递
            if(e->source() == this)
            {
                e->setDropAction(Qt::MoveAction);
                e->accept();
            }else {
                e->acceptProposedAction();
            }
        }
        else{
            qInfo() << "Mould not implemented";
            e->ignore();
        }
    };
public:
    ///在FlowScene中创建node
    void createNode(QString const &name,QString const &caption,const QPoint pos) const
    {
            QPointF posView = this->mapToScene(pos);
            _scene->dropCreateNode(name,caption,posView);
    }

    ///view的缩放复原为原始大小
    void scaleDefault(){
        this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        QMatrix q;
        q.setMatrix(1,this->matrix().m12(),this->matrix().m21(),1,this->matrix().dx(),this->matrix().dy());
        this->setMatrix(q,false);
    }

    AICCFlowScene *scene() const{
        return _scene;
    }

private:
    AICCFlowScene* _scene;
};

//}
#endif // AICCFLOWVIEW_H
