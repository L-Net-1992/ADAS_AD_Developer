#ifndef MODELS_RECENT_PROJECT_H
#define MODELS_RECENT_PROJECT_H

#include <QtCore/QObject>
#include <QJsonObject>
#include <QMap>
#include <QFile>
#include <QDomDocument>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
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

    ///读取平台的配置信息
    void readPlatformConfig(){

        QString platformPath = QApplication::applicationDirPath().append("/platform.xml");
        QFile pfile(platformPath);
        QDomDocument doc;
        //如果没有平台项目文件则创建一个新的
        if(!pfile.exists()){
            pfile.open(QIODevice::ReadWrite | QIODevice::Text);
            QDomProcessingInstruction ins;
            ins = doc.createProcessingInstruction("xml","version=\'1.0\' encoding=\'UTF-8\'");
            doc.appendChild(ins);
            QDomElement root = doc.createElement("platform");
            QDomElement projects = doc.createElement("projects");
            root.appendChild(projects);
            doc.appendChild(root);

            QTextStream stream(&pfile);
            stream.setCodec("UTF-8");

            doc.save(stream,4);         //4表示缩进的空格数
            pfile.close();
        }

        //打开平台项目文件
        if(pfile.open(QIODevice::ReadOnly) && doc.setContent(&pfile)){
            QDomElement root = doc.documentElement();
            QDomNodeList nlist = root.elementsByTagName("projects");
            QDomNode nProjects = nlist.at(0);

            QDomNodeList nProject = nProjects.childNodes();

            for(int i=0;i<nProject.count();i++){
                QString pname = nProject.at(i).toElement().attributeNode("name").value();
                QString ppath = nProject.at(i).toElement().attributeNode("path").value();
                QSharedPointer<ProjectDataModel> pdm(new ProjectDataModel());
                pdm->setProject(pname,ppath);
                addRecentProject(pname,pdm);
            }
        }



    }

    ///将参数中的项目排序到第一位
    void sortProjectFirst(const QString pname,const QSharedPointer<ProjectDataModel> pdm){
        QMap<QString,QSharedPointer<ProjectDataModel>> map;
        map.insert(pname,pdm);
        _recentProjects.insert(0,map);

        QString platformPath = QApplication::applicationDirPath().append("/platform.xml");
        QFile pfile(platformPath);
        QDomDocument doc;

        //打开平台项目文件
        if(pfile.open(QIODevice::ReadOnly) && doc.setContent(&pfile)){
            QDomElement root = doc.documentElement();
            QDomNodeList nlist = root.elementsByTagName("projects");
            QDomNode nProjects = nlist.at(0);

            QDomNodeList nProject = nProjects.childNodes();

            for(int i=0;i<nProject.count();i++){
                nProjects.removeChild(nProject.at(i));
                qDebug() << "nProject " << i;
            }

//            nProjects.removeChild()
//            nProjects.childNodes().at(0).

//            QDomNodeList nProject = nProjects.childNodes();


        }

    }

    ///判断项目是否存在
    bool existProject(const QString ppath){
        QDir existDir(ppath);
        QFile existFile(ppath+"/.ap/project.xml");
        if(!existDir.exists() || !existFile.exists()) return false;
        return true;
    }
private:

    ///最近访问的项目 QMap的key为项目名称，value为项目的路径
    QList<QMap<QString,QSharedPointer<ProjectDataModel>>> _recentProjects;
};


#endif // MODELS_RECENT_PROJECT_H
