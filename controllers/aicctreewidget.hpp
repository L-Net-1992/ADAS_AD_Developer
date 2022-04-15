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


    ///初始化左侧功能树数据,主界面使用
    void fillInitLeftTreeData(){

        this->setDragDropMode(QAbstractItemView::DragOnly);
        this->setDragEnabled(true);
        this->clear();

        Config config(QApplication::applicationDirPath()+"/conf/model_menu.json");
        QJsonObject jo_root = config.getJsonRoot();
        QList<QPair<QString,QJsonObject>> list_root = orderedQJsonObject(jo_root);

        for(int i=0;i<list_root.size();i++){
            QTreeWidgetItem *twi_root = new QTreeWidgetItem(this);
            twi_root->setText(0,list_root[i].first);
            //        twi->setBackground(0,QBrush(QColor("#FFFFFF")));
            recursionQJsonObjectLeft(list_root[i].first,list_root[i].second,twi_root);
        }

        refreshTreeViewDynamicNode();
        this->expandAll();
        //        clearDynamicNode();
    }

private:
    ///将数据库中的动态节点加到功能树中
    void refreshTreeViewDynamicNode(){
        //根据当前每个节点的自定义数据，先清理调所有的自定义数据
        clearDynamicNode();

        QList<QPair<QString,QJsonObject>> list_root = orderedQJsonObject(Config(QApplication::applicationDirPath()+"/conf/model_menu.json").getJsonRoot());
        //处理动态节点
        for(int i=0;i<list_root.size();i++){
            if(list_root[i].first=="应用软件" || list_root[i].first=="自定义模块"){
                AICCSqlite sqlite;
                QString sql = QString("select nd.name,nd.caption,nd.class_id,nc.class_name from nodeDynamic nd inner join nodeClass nc on nc.id = nd.class_id where nc.class_name='%0'").arg(list_root[i].first);
                QSqlQuery query = sqlite.query(sql);

                QList<QTreeWidgetItem*> ltwi = this->findItems(list_root[i].first,Qt::MatchFixedString | Qt::MatchRecursive);
                while(query.next()){
                    QTreeWidgetItem *twi_dynamic = new QTreeWidgetItem(ltwi.at(0));
                    twi_dynamic->setWhatsThis(0,"dynamic_node");
                    twi_dynamic->setText(0,query.value(1).toString());
                    twi_dynamic->setData(0,Qt::UserRole+1,QVariant::fromValue(query.value(0).toString()));
                    //                    qDebug() << "应用软件 ltwi[i]:" << ltwi[0]->text(0) << "  list_root[i]" << list_root[i].first<< twi_dynamic->text(0);
                }
            }
            else
                continue;
        }
    }
    ///更新动态节点的位置
    void updateDynamicNode(int objectClassID,QString caption,int currentClassID){
        AICCSqlite sqlite;
        QString sql = QString("update nodeDynamic set class_id = %0 where caption = '%1' and class_id = %2 ")
                .arg(objectClassID).arg(caption).arg(currentClassID);
        QSqlQuery query;
        //        QSqlQuery query = sqlite.query(sql);
        query.exec(sql);
        this->refreshTreeViewDynamicNode();
    }

    ///清理所有动态节点
    void clearDynamicNode(){
        QVector<QTreeWidgetItem*> vector;
        QTreeWidgetItemIterator it(this);
        while(*it){
            if((*it)->whatsThis(0)=="dynamic_node"){
                vector.append(*it);
            }
            ++it;
        }

        for(int i=0;i<vector.count();i++){
            QTreeWidgetItem *removeItem = vector.at(i);
            QTreeWidgetItem *pwi = removeItem->parent();
            pwi->removeChild(removeItem);

        }
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

                if(pname == "自定义模块" || pname == "应用软件"){
                    AICCSqlite sqlite;
                    QString sql = QString("select nd.name,nd.class_id,nc.class_name from nodeDynamic nd inner join nodeClass nc on nd.class_id = nc.id where nc.class_name = '%0' and nd.caption = '%1'").arg(pname,item->text(0));
                    QSqlQuery query = sqlite.query(sql);
                    if(query.next()){
                        QMenu menu;
                        menu.addAction(tr("设置到应用软件"),this,[&]{
                            updateDynamicNode(37,item->text(0),query.value(1).toInt());
                        });
                        menu.addAction(tr("设置到自定义模块"),this,[&]{
                            updateDynamicNode(38,item->text(0),query.value(1).toInt());
                        });
                        menu.exec(QCursor::pos());
                    }
                }
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
