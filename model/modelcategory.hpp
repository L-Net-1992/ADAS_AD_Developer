//
// Created by 冯冲 on 2022/3/24.
//


#ifndef MODELS_CATEGORY_H
#define MODELS_CATEGORY_H

#include <QtCore/QObject>
#include <QJsonObject>
#include <QMap>
#include <QFile>
#include <QDomDocument>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDebug>
#include "sqlite/aiccsqlite.hpp"

class CategoryDataModel:public QObject{
    Q_OBJECT
public:
    CategoryDataModel(){
        _category = recursionChildren(_category,0);
        emit dataLoadCompleted(_category);
    }
    const QJsonObject &category() const;

    void refreshCategoryDataModel(){
        emit dataLoadCompleted(recursionChildren(_category,0));
    }

Q_SIGNALS:
    void dataLoadCompleted(const QJsonObject json);


private:
    /// 递归所有子节点
    /// sqlquery不能在递归中使用，此处将结果缓存到QVector<QMap<QString,QVariant>>中使用
    QJsonObject recursionChildren(QJsonObject node,int pid){
        AICCSqlite sqlite;
        QString sql = QString("select id,parentid,class_name,caption,is_node,icon_name from modelNode where parentid = %0").arg(pid);

        QVector<QMap<QString,QVariant>> vector = sqlite.query1(sql,
                                                                [](QSqlQuery q){
                QMap<QString,QVariant> v;
                v.insert("id",q.value("id").toInt());
                v.insert("parentid",q.value("parentid").toInt());
                v.insert("caption",q.value("caption").toString());
                v.insert("is_node",q.value("is_node").toInt());
                v.insert("icon_name",q.value("icon_name").toString());
                return v;
        });

        QVectorIterator vit(vector);
        while(vit.hasNext()){
            const QMap<QString,QVariant> m = vit.next();
            int id = m.value("id").toInt();
            QString caption = m.value("caption").toString();
            QString iconName = m.value("icon_name").toString();
            int isNode = m.value("is_node").toInt();


            if(isNode==0){
                QJsonObject childNode;
                childNode.insert("id",id);
                childNode.insert("caption",caption);
                childNode.insert("icon_name",iconName);
                childNode.insert("is_node",isNode);

                node.insert(QString::number(id),recursionChildren(childNode,id));
            }
        }
        return node;
    }

private:
    QJsonObject _category;
};


inline const QJsonObject &CategoryDataModel::category() const
{
    return _category;
}



#endif // MODELS_CATEGORY_H

