#ifndef AICCTOOLBUTTON_H
#define AICCTOOLBUTTON_H

#include <QToolButton>
#include <QMouseEvent>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QStyleFactory>
#include <QStyle>

class AICCToolButton : public QToolButton
{
    Q_OBJECT
public:
    AICCToolButton(QWidget *parent=nullptr):QToolButton(parent){
        //设置文字与图标的布局
        QString pbt_style = "background-color:white;text-align:bottom;background-repeat:repeat-no-repeat;background-position:top;background-origin:content;";
        this->setStyleSheet(pbt_style);
//        this->setStyleSheet(this->styleSheet()+"background-image:url(:/res/nodeIcon/math_add.png);");
        QSize qsize(45,45);
        this->setIconSize(qsize);
        this->setMinimumSize(135,90);
        QSizePolicy sp = this->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Preferred);
        this->setSizePolicy(sp);
        this->setAttribute(Qt::WA_TransparentForMouseEvents,true);


    };
    ///设置Node的Icon
    void setNodeIcon(const QString iconUrl){
//        this->setStyleSheet(this->styleSheet()+"background-image:url(:"+iconUrl+");");
        QIcon icon(iconUrl);
        this->setIcon(icon);
    }
    void setNodeId(QString id){_nodeId = id;}
    void setNodeName(QString name){_nodeName = name;}
    void setNodeCaption(QString caption){_nodeCaption = caption;}
    void setNodeParentId(const QString &newNodeParentId){_nodeParentId = newNodeParentId;}
    const QString &nodeName() const;
    const QString &nodeId() const;
    const QString &nodeParentId() const;
    const QString &nodeCaption() const;


protected:


    void mousePressEvent(QMouseEvent *e)
    {
        if(e->button() & Qt::LeftButton){
            QByteArray dataItemName;
            QDataStream dataStreamName(&dataItemName,QIODevice::WriteOnly);
            dataStreamName << _nodeName;

            QByteArray dataItemCaption;
            QDataStream dataStreamCaption(&dataItemCaption,QIODevice::WriteOnly);
            QString sidataCaption = _nodeCaption;
            dataStreamCaption << sidataCaption;

            QByteArray dataItemId;
            QDataStream dataStreamId(&dataItemId,QIODevice::WriteOnly);
            QString dataId = _nodeId;
            dataStreamId << dataId;

            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData();
            mimeData->setData("Data/name",dataItemName);
            mimeData->setData("Data/caption",dataItemCaption);
            mimeData->setData("Data/id",dataItemId);

            drag->setMimeData(mimeData);
            drag->exec(Qt::MoveAction);
        }

//        qDebug() << "mouse press event" << dataItem << " "<< dataItemName;
        e->accept();
    }
    void mouseReleaseEvent(QMouseEvent *e)
    {
//        qDebug() << "mouse release event";
        e->accept();
    };
    void mouseMoveEvent(QMouseEvent *e)
    {
//        qDebug() << "mouse move event";
        e->accept();
    };

signals:
    void preperNewTestButton();
private slots:

private:
    QPoint _pressPoint;
    QString _nodeName;
    QString _nodeCaption;
    QString _nodeId;
    QString _nodeParentId;

};

inline const QString &AICCToolButton::nodeName() const
{
    return _nodeName;
}

inline const QString &AICCToolButton::nodeId() const
{
    return _nodeId;
}

inline const QString &AICCToolButton::nodeParentId() const
{
    return _nodeParentId;
}

inline const QString &AICCToolButton::nodeCaption() const
{
    return _nodeCaption;
}

#endif // AICCTOOLBUTTON_H
