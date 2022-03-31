#ifndef MODELS_RECENT_PROJECT_H
#define MODELS_RECENT_PROJECT_H

#include <QtCore/QObject>
#include <QJsonObject>
#include <QMap>
#include <QFile>
#include <QDomDocument>
#include <QMessageBox>
#include "project/modelsproject.hpp"

class RecentProjectDataModel:public QObject{
    Q_OBJECT
public:
    RecentProjectDataModel(){

    }
    ~RecentProjectDataModel(){}

Q_SIGNALS:
    void addRecentProjectEvent(const QString name, const QSharedPointer<ProjectDataModel> pdm);

public:
    void addRecentProject( QString pname, QSharedPointer<ProjectDataModel> pdm){
        QMap<QString,QSharedPointer<ProjectDataModel>> map;
        map.insert(pname,pdm);
        _recentProjects.append(map);
        emit addRecentProjectEvent(pname,pdm);
    };

private:

    ///最近访问的项目 QMap的key为项目名称，value为项目的路径
    QList<QMap<QString,QSharedPointer<ProjectDataModel>>> _recentProjects;
};


#endif // MODELS_RECENT_PROJECT_H
