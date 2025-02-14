﻿//
// Created by 冯冲 on 2022/3/24.
//


#ifndef MODELS_CATEGORY_H
#define MODELS_CATEGORY_H

#include <QtCore/QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QFile>
#include <QDomDocument>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDebug>
#include "sqlite/aiccsqlite.hpp"
#include "ADAS_AD_Backend/module_library.hpp"
#include "ADAS_AD_Backend/subsystem_library.h"

class CategoryDataModel:public QObject{
    Q_OBJECT
public:
    CategoryDataModel(){
        //        _category = recursionChildren(_category,0);
        //        qDebug() << "make category full path:" << makeCategoryFullPath(7);
        //        emit dataLoadCompleted(_category);

    }
    const QJsonObject &category() const;

    /**
     * @brief refreshCategoryDataModel  刷新分类数据，刷新分类数据后同时要刷新已加载的node数据
     * @param ml
     * @param sl
     */
    void refreshCategoryDataModel(ModuleLibrary &ml){

        _currentLoadedNode = makeCurrentLoadedNode(ml);
        _currentUseCategoryFullPath = makeAllCategoryFullPath(_currentLoadedNode);
        ml.setCurrentUseCategoryFullPath(_currentUseCategoryFullPath);
        QJsonObject json = recursionChildren(_category,0);

        emit dataLoadCompleted(json);

    }

    /**
     * @brief makeCategoryFullPath  给定功能模块id，获得该功能模块的分类路径
     * @param nid                   模块id
     * @return                      返回模块id所在分类的路径
     */
    std::pair<int,std::string> makeCategoryFullPath(const int nid){
        std::pair<int,std::string> retfp ;
        AICCSqlite sqlite;
        QString sql = QString("select id,parentid,class_name from modelNode where id = %0 and is_node = %1").arg(nid).arg(1);
        QSqlQuery query = sqlite.query(sql);
        if(query.next()){
            int id = query.value("id").toInt();
            QString categoryFullPath = getParentDataByChildid("",id);
            retfp.first = id;
            retfp.second = QString(categoryFullPath).toStdString();

        }
        return retfp;
    }


    /**
     * @brief makeCategoryFullPath  给定功能模块名称，获得该功能模块的分类路径
     * @param cn                    模块名称class_name
     * @return                      返回模块class_name所在分类的路径
     */
    std::pair<int,std::string> makeCategoryFullPath(const QString cn){
        std::pair<int,std::string> retfp;
        AICCSqlite sqlite;
        QString sql = QString("select id,parentid,class_name from modelNode where class_name = '%0' and is_node = %1").arg(cn).arg(1);
        QSqlQuery query = sqlite.query(sql);
        if(query.next()){
            int id = query.value("id").toInt();
            int parentid = query.value("parentid").toInt();
            QString categoryFullPath = getParentDataByChildid("",id);
            retfp.first = parentid;
            retfp.second = QString(categoryFullPath).toStdString();
        }
        return retfp;
    }

    /**
     * @brief makeAllCategoryFullPath   获得所有的完整路径
     * @param cln                       所有使用的节点
     * @return                          以list形式返回所有可选择的完整路径
     */
    std::vector<std::pair<int,std::string>> makeAllCategoryFullPath(std::vector<std::string> cln){
//        QSet<QString> set;
        std::vector<std::pair<int,std::string>> retacfp;
        std::set<std::string> setacfp;
        for(std::string n:cln){
            std::pair<int,std::string> category = makeCategoryFullPath(QString::fromStdString(n));
            if(setacfp.count(category.second)==0){
                setacfp.insert(category.second);
                retacfp.push_back(category);
            }
        }

        //排序操作
        qSort(retacfp.begin(),retacfp.end(),[](const std::pair<int,std::string> &s1,const std::pair<int,std::string> &s2){
            return s1.second < s2.second;
        });

        return retacfp;
    }

    /**
     * @brief existNode 判断给定名称的node在当前的moduleLibrary、subsystemLibrary中是否存在
     * @param nn        模块名称
     * @return          返回是否存在
     */
    bool existNode(std::string nn){
        std::vector<std::string>::iterator it;
        it = std::find(_currentLoadedNode.begin(),_currentLoadedNode.end(),nn);
        if(it != _currentLoadedNode.end())
            return true;
        return false;
    }

    const std::vector<std::pair<int,std::string>> &currentUseCategoryFullPath() const{
       return _currentUseCategoryFullPath;
    };
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


    /**
     * @brief recursionChildren 递归所有子节点，最终以树形结构数据表达出来
     * @param node              当前json数据，作为父数据用于获得子分类数据
     * @param pid               当前要检索的父节点内容的父节点id
     * @return                  返回带有当前父id子节点内容
     */
    QJsonObject recursionChildren(QJsonObject node,int pid){
        AICCSqlite sqlite;
        QString sql = QString("select id,parentid,class_name,caption,is_node,icon_name,sort_same_parentid from modelNode where parentid = %0 order by sort_same_parentid").arg(pid);

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

        QJsonArray jIdSort;
        QVectorIterator vit(vector);
        while(vit.hasNext()){
            const QMap<QString,QVariant> m = vit.next();
            int id = m.value("id").toInt();
            QString caption = m.value("caption").toString();
            QString iconName = m.value("icon_name").toString();
            int isNode = m.value("is_node").toInt();

            jIdSort.append(id);

            if(isNode==0){
                QJsonObject childNode;
                childNode.insert("id",id);
                childNode.insert("caption",caption);
                childNode.insert("icon_name",iconName);
                childNode.insert("is_node",isNode);

                node.insert("childSort",jIdSort);
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
    std::vector<std::string> makeCurrentLoadedNode(ModuleLibrary &ml){
        std::list<Invocable> l_result = ml.getParseResult();
        std::list<Invocable>::iterator it_module_inv;
        std::vector<Invocable> l_subsystem_result = ml.subsystemLibrary().getInvocableList();
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
    std::vector<std::string> _currentLoadedNode;                            //当前已经加载的所有node
    std::vector<std::pair<int,std::string>> _currentUseCategoryFullPath;   //当前完整的分类路径的id，与完整分类路径字符串描述
};


inline const QJsonObject &CategoryDataModel::category() const
{
    return _category;
}



#endif // MODELS_CATEGORY_H

