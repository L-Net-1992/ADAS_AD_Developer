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
        this->setAcceptDrops(true);
        this->setDragMode(QGraphicsView::DragMode::NoDrag);

        initShortcut(parent);

        //模块右键菜单
        connect(_scene,&AICCFlowScene::nodeContextMenu,this,&AICCFlowView::nodeContextMenuAction);

    }
    AICCFlowView(QWidget *parent):FlowView(parent){

    }


protected:
    ///此函数不能删除，删除后拖放功能不好使
    void dragMoveEvent(QDragMoveEvent *e)
    {
        //        qDebug()<<"flow view drag move";
        //        e->accept();
    };
    void dropEvent(QDropEvent *e) {
        QStringList formats = e->mimeData()->formats();
        qDebug() << "e->mimeData()" << e->mimeData();
        if(e->mimeData()->hasFormat("Data/name"))
        {
            QByteArray itemData = e->mimeData()->data("Data/name");
            QDataStream dataStream(&itemData,QIODevice::ReadOnly);
            QString name;
            dataStream >> name;

            QByteArray itemCaptionData = e->mimeData()->data("Data/caption");
            QDataStream dataCaptionStream(&itemCaptionData,QIODevice::ReadOnly);
            QString caption;
            dataCaptionStream >> caption;

            qInfo() << "info:" << name << " " << caption;

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

    void actionCopy(){
        QClipboard *clipboard = QApplication::clipboard();
        std::vector<Node*> nodes =  _scene->selectedNodes();
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
        QPointF pos = this->mapFromScene(QCursor::pos());

        for(int i=0;i<sl.size();i++){
            QString nn = sl.at(i);
            QPointF p(pos.x()+(i*10),pos.y()+(i*10));
            _scene->dropCreateNode(nn,nn,p);
        }
    }

private:
    void initShortcut(QWidget *parent){

        _sc_copy = new QShortcut(QKeySequence::Copy,parent);
        _sc_paste = new QShortcut(QKeySequence::Paste,parent);


        connect(_sc_copy,&QShortcut::activated,this,[&](){
            qDebug() << "copy";
        });
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
        connect(act_copy,&QAction::triggered,this,&AICCFlowView::actionCopy);

        //粘贴
        connect(act_paste,&QAction::triggered,this,&AICCFlowView::actionPaste);

        //重命名
        connect(act_rename,&QAction::triggered,this,[&](){
            emit nodeRename(node);
        });



        pop_menu->exec(QCursor::pos());

        //        //回收action资源
        foreach(QAction *pAction,pop_menu->actions()) delete pAction;
    }

Q_SIGNALS:
    //该信号需要将view对象以参数方式传递过去使用,否则会有问题
    void checkSubSystemName(const QString &name,const QString &caption,const QPoint mousePos,const AICCFlowView *cview);
    void nodeRename(Node &node);



private:
    AICCFlowScene* _scene;
    QShortcut *_sc_copy;
    QShortcut *_sc_paste;
    QShortcut *_sc_rename;
};

//}
#endif // AICCFLOWVIEW_H
