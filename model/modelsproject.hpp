//
// Created by 冯冲 on 2022/3/24.
//


#ifndef MODELS_PROJECT_H
#define MODELS_PROJECT_H

#include <QtCore/QObject>
#include <QJsonObject>
#include <QMap>
#include <QFile>
#include <QDomDocument>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>

class ProjectDataModel:public QObject{
    Q_OBJECT
public:
    ProjectDataModel(){

    }
    ~ProjectDataModel(){}
Q_SIGNALS:
    void projectDataModelLoadCompleted(const QString pname,const QString ppath);

private:
    ///当前打开的项目名称
    QString _projectName;
    ///当前打开的项目路径
    QString _projectPath;
    ///当前项目的子系统路径
    QString _projectSubSystemPath;
    ///项目信息xml文件
    QString _projectXml;
    ///当前项目的所有flowScene数据
    QStringList _flowSceneSaveFiles;


public:
    const QString &projectName() const{return _projectName;}
    const QString &projectPath() const{return _projectPath;}
    const QString &projectSubSystemPath() const{return _projectSubSystemPath;}
    const QStringList &flowSceneSaveFiles() {return _flowSceneSaveFiles;}
    void setProject(const QString pname,const QString ppath){
        const QFileInfo fileInfo(ppath);
        setProject(fileInfo);
    }
    /// 设置项目信息
    /// 通过设置项目名称、路径可以获得项目的名称、路径、子目录、项目文件目录、项目文件内容
    void setProject(const QFileInfo pFileInfo){
        _projectName = pFileInfo.baseName();
        _projectPath = pFileInfo.absoluteFilePath();
        _projectSubSystemPath = _projectPath+"/subsystem";
        _projectXml = _projectPath+"/project.xml";
        parseProjectXml(_projectXml);
        emit projectDataModelLoadCompleted(_projectName,_projectPath);
    }

private:
    ///解析项目的配置文件
    bool parseProjectXml(const QString pxml){
        QFile file(pxml);

        QDomDocument doc;

        if(!doc.setContent(&file)){
            file.close();
            return false;
        }

        //获取项目下所有的flowScene文件
        QDomElement root = doc.documentElement();
        QDomNodeList flowScenes = root.childNodes().item(0).childNodes();
        _flowSceneSaveFiles.clear();
        for(int i=0;i<flowScenes.count();i++){
            QDomNode flowScene = flowScenes.item(i);
            _flowSceneSaveFiles.append(flowScene.toElement().attributeNode("saveFile").value());
        }
        return true;
    }
};


#endif // MODELS_PROJECT_H
