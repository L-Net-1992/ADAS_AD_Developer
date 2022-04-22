#ifndef AICCTREEWIDGET_H
#define AICCTREEWIDGET_H

#include <QTreeWidget>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDebug>
#include <QSplitter>
#include <QMimeData>
#include <QDrag>
#include <QMenu>
#include <QVBoxLayout>
#include <conf/Config.hpp>
#include <QApplication>
#include "utils.h"
#include <QStandardItem>


class AICCTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    AICCTreeWidget(QWidget *parent=nullptr)
    {
        connect(this,&QTreeWidget::customContextMenuRequested,this,&AICCTreeWidget::onTreeWidgetCustomContextMenuRequested);
    }

protected:
    /**
     * 鼠标按下操作
     * @brief mousePressEvent
     * @param event
     */
    void mousePressEvent(QMouseEvent* event)
    {
        if(event->button() & Qt::LeftButton)
        {
            _selectItem = itemAt(event->pos());
            if(_selectItem==Q_NULLPTR) return;

            QByteArray dataItem;
            QDataStream dataStream(&dataItem,QIODevice::WriteOnly);
            dataStream << _selectItem->text(0);

            QByteArray dataItemName;
            QDataStream dataStreamName(&dataItemName,QIODevice::WriteOnly);
            QString sidata = _selectItem->data(0,Qt::UserRole+1).toString();
            dataStreamName << sidata;

            if(_selectItem!=Q_NULLPTR && _selectItem->parent()!=Q_NULLPTR &&QString::compare(sidata,"")!=0)
            {
                QDrag* drag = new QDrag(this);

                QMimeData* mimeData = new QMimeData;
                mimeData->setData("Data/caption",dataItem);
                //将要生成的flowview节点的名字代入Data/name字段
                mimeData->setData("Data/name",dataItemName);

                //            QtGuiDrag* dragPiamap = new QtGuiDrag(nullptr);
                //            dragPiamap->setShowText(_selectItem->text(0));
                //            QPixmap pixmap = dragPiamap->grab();

                drag->setMimeData(mimeData);
                drag->exec(Qt::MoveAction);
            }
        }
        //        else if(event->button() && Qt::RightButton){
        QTreeWidget::mousePressEvent(event);
        //        }

    };
    /**
     * 自定义鼠标右键函数
     * @brief onTreeWidgetCustomContextMenuRequested
     * @param pos
     */
    void onTreeWidgetCustomContextMenuRequested(const QPoint &pos)
    {
        QModelIndex curIndex = this->indexAt(pos);

        if(curIndex.isValid()){

            QTreeWidgetItem *item = this->itemAt(pos);
            if(item->parent()!=Q_NULLPTR){
                QString pname = item->parent()->text(0);

//                if(pname == "自定义模块" || pname == "应用软件"){
//                    AICCSqlite sqlite;
//                    QString sql = QString("select nd.name,nd.class_id,nc.class_name from nodeDynamic nd inner join nodeClass nc on nd.class_id = nc.id where nc.class_name = '%0' and nd.caption = '%1'").arg(pname,item->text(0));
//                    QSqlQuery query = sqlite.query(sql);
//                    if(query.next()){
//                        QMenu menu;
//                        menu.addAction(tr("设置到应用软件"),this,[&]{
//                            updateDynamicNode(37,item->text(0),query.value(1).toInt());
//                        });
//                        menu.addAction(tr("设置到自定义模块"),this,[&]{
//                            updateDynamicNode(38,item->text(0),query.value(1).toInt());
//                        });
//                        menu.exec(QCursor::pos());
//                    }
//                }
            }
        }
    }

    ///此处需重载dragEnterEvent函数，否则dropEvent函数不会响应
    void dragEnterEvent(QDragEnterEvent* event){
        //        qDebug() << "drag enter event:" << event;
        //此处需要执行以下两句，否则dropEvent不会响应
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }

    ///放置函数
    void dropEvent(QDropEvent *event){
        const QMimeData *mdata = event->mimeData();
        mdata->data("Data/name");
        mdata->data("Data/caption");


        QByteArray itemDataId = mdata->data("Data/id");
        QDataStream dataStreamId(&itemDataId,QIODevice::ReadOnly);
        QString id;
        dataStreamId >> id;

        QTreeWidgetItem * item = this->itemAt(event->pos());
        QVariant v = item->data(0,Qt::UserRole+1);
        int pid = v.toJsonObject().value("id").toInt();

        AICCSqlite sqlite;
        QString sql = QString("update modelNode set parentid = %0 where id = %1").arg(pid).arg(id);
        sqlite.query(sql);

        //将当前选择的item传送出去，用来刷新当前目录下的模块列表
        emit dropCompleted(_selectItem);
    }

public:
Q_SIGNALS:
    void dropCompleted(QTreeWidgetItem * item);



private:
    QTreeWidgetItem* _selectItem;
};

#endif // AICCTREEWIDGET_H
