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
#include "nodeparser/module_library.hpp"
#include "nodeparser/subsystem_library.h"

class CategoryDataModel:public QObject{
    Q_OBJECT
public:
    CategoryDataModel(){
        _category = recursionChildren(_category,0);
        qDebug() << "make category full path:" << makeCategoryFullPath(7);
        emit dataLoadCompleted(_category);

    }
    const QJsonObject &category() const;

    /**
     * @brief refreshCategoryDataModel  刷新分类数据，刷新分来数据后同时要刷新已加载的node数据
     * @param ml
     * @param sl
     */
    void refreshCategoryDataModel(QSharedPointer<ModuleLibrary> ml,QSharedPointer<SubsystemLibrary> sl){

        _currentUseNode = makeCurrentLoadedNode(ml,sl);
        emit dataLoadCompleted(recursionChildren(_category,0));
    }

    /**
     * @brief makeCategoryFullPath  给定功能模块id，获得该功能模块的分类路径
     * @param nid                   模块id
     * @return                      返回模块id所在分类的路径
     */
    QString makeCategoryFullPath(const int nid){
        AICCSqlite sqlite;
        QString sql = QString("select id,parentid,class_name from modelNode where id = %0 and is_node = %1").arg(nid).arg(1);
        QSqlQuery query = sqlite.query(sql);
        if(query.next()){
            int id = query.value("id").toInt();
            QString categoryFullPath = getParentDataByChildid("",id);
            return categoryFullPath;
        }else{
//            qWarning() << "未查询到id为:" << nid << "的模块，请确认id正确";
            return "";
        }
    }

    /**
     * @brief makeCategoryFullPath  给定功能模块名称，获得该功能模块的分类路径
     * @param cn                    模块名称class_name
     * @return                      返回模块class_name所在分类的路径
     */
    QString makeCategoryFullPath(const QString cn){
        AICCSqlite sqlite;
        QString sql = QString("select id,parentid,class_name from modelNode where class_name = '%0' and is_node = %1").arg(cn).arg(1);
        QSqlQuery query = sqlite.query(sql);
        if(query.next()){
            int id = query.value("id").toInt();
            QString categoryFullPath = getParentDataByChildid("",id);
            return categoryFullPath;
        }else{
//            qWarning() << "数据库中未查询到名称为:" << cn << "的模块，请确认模块名称正确";
            return "";
        }
    }

    /**
     * @brief existNode 判断给定名称的node在当前的moduleLibrary、subsystemLibrary中是否存在
     * @param nn        模块名称
     * @return          返回是否存在
     */
    bool existNode(std::string nn){
        std::vector<std::string>::iterator it;
        it = std::find(_currentUseNode.begin(),_currentUseNode.end(),nn);
        if(it != _currentUseNode.end())
            return true;
        return false;
    }

Q_SIGNALS:
    void dataLoadCompleted(const QJsonObject json);


private:
    /**
     * @brief getParent         通过父id获得当前父节点下一级的子分类数据
     * @param categoryFullPath  分类的完整路径数据，添加当前的子分类数据后返回
     * @param id                当前节点模块的id
     * @return
     */
    QString getParentDataByChildid(QString categoryFullPath,int id){

       AICCSqlite sqlite;
       QString sql = QString("select mnp.id,mnp.class_name,mnp.parentid,mnp.caption from modelNode mnp inner join modelNode mn on mnp.id = mn.parentid where mn.id = %0").arg(id);
       QSqlQuery query = sqlite.query(sql);

       if(query.next()){
           int id = query.value("id").toInt();
           QString cname = query.value("class_name").toString();
           QString caption = query.value("caption").toString();

           categoryFullPath = categoryFullPath!=""?caption.append("|").append(categoryFullPath):caption;
           return getParentDataByChildid(categoryFullPath,id);
       }
       else
           return categoryFullPath;
    }


    /// 递归所有子节点
    /// sqlquery不能在递归中使用，此处将结果缓存到QVector<QMap<QString,QVariant>>中使用
    /**
     * @brief recursionChildren 递归所有子节点，最终以树形结构数据表达出来
     * @param node              当前json数据，作为父数据用于获得子分类数据
     * @param pid               当前要检索的父节点内容的父节点id
     * @return                  返回带有当前父id子节点内容
     */
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

    /**
     * @brief makeCurrentUseNode    生成现在ModuleLbrary与SubsystemLibrary中
     * @param ml                    模型库参数
     * @param sl                    子系统库参数
     * @return                      返回
     */
    std::vector<std::string> makeCurrentLoadedNode(QSharedPointer<ModuleLibrary> ml,QSharedPointer<SubsystemLibrary> sl){
        std::list<Invocable> l_result = ml->getParseResult();
        std::list<Invocable>::iterator it_module_inv;
        std::vector<Invocable> l_subsystem_result = sl->getInvocableList();
        std::vector<Invocable>::iterator it_subsystem_inv;
        std::vector<std::string> l_result_cname;
        for(it_module_inv = l_result.begin();it_module_inv != l_result.end();++it_module_inv)
            l_result_cname.push_back(it_module_inv->getName());
        for(it_subsystem_inv = l_subsystem_result.begin();it_subsystem_inv != l_subsystem_result.end();++it_subsystem_inv)
            l_result_cname.push_back(it_subsystem_inv->getName());

        return l_result_cname;
    }



private:
    QJsonObject _category;
    QMap<int,QString> _categoryFullPath;
    std::vector<std::string> _currentUseNode;
};


inline const QJsonObject &CategoryDataModel::category() const
{
    return _category;
}



#endif // MODELS_CATEGORY_H

