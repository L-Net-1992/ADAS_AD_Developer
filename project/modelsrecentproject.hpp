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

    /// 将参数中的项目排序到第一位
    /// 排序操作会重写platform.xml文件
    void sortProjectFirst(const QString pname,const QSharedPointer<ProjectDataModel> pdm){
        QMap<QString,QSharedPointer<ProjectDataModel>> map;
        map.insert(pname,pdm);
        _recentProjects.insert(0,map);

        QString platformPath = QApplication::applicationDirPath().append("/platform.xml");
        QFile pfile(platformPath);
        QDomDocument doc;

        //打开平台项目文件
        if(pfile.open(QIODevice::ReadWrite) && doc.setContent(&pfile)){
            QDomElement root = doc.documentElement();
            QDomNodeList nlist = root.elementsByTagName("projects");
            QDomNode nProjects = nlist.at(0);

            QDomNodeList nProject = nProjects.childNodes();

            //1：判断当前pname的节点是否存在
            int isProjectExist = -1;
            for(int i=0;i<nProject.count();i++){
                if(pname==nProject.at(i).attributes().namedItem("name").nodeValue()){
                    isProjectExist = i;
                    break;
                }
            }

            //2:不存在新增，存在则调整顺序
            if(isProjectExist==-1){
                QDomElement newProject = doc.createElement("project");
                QDomAttr newNameAttr = doc.createAttribute("name");
                newNameAttr.setValue(pname);
                QDomAttr newPathAttr = doc.createAttribute("path");
                newPathAttr.setValue(pdm->projectPath());
                newProject.setAttributeNode(newNameAttr);
                newProject.setAttributeNode(newPathAttr);
                nProjects.insertBefore(newProject,nProject.at(0));
            }else{
                QDomNode n = nProject.at(isProjectExist);
                nProjects.insertBefore(n,nProjects.firstChild());
            }

            //写入数据
            pfile.close();
            pfile.setFileName(platformPath);
            if(pfile.open(QIODevice::WriteOnly)){
                QTextStream outStream(&pfile);
                doc.save(outStream,4);
                pfile.flush();
                pfile.close();
            }
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
