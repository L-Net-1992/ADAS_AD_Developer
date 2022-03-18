#ifndef MODELS_PROJECT_H
#define MODELS_PROJECT_H

#include <QtCore/QObject>
#include <QJsonObject>
#include <QMap>
#include <QFile>
#include <QDomDocument>
#include <QMessageBox>

class ProjectDataModel:public QObject{
    Q_OBJECT
public:
    ProjectDataModel(){

    }
    ~ProjectDataModel(){}

Q_SIGNALS:
    void addRecentProjectEvent(const QString name, const QString path);

private:
    ///当前打开的项目名称
    QString _currentProjectName;
    ///当前打开的项目路径
    QString _currentProjectPath;
    ///当前项目的子系统路径
    QString _currentProjectSubSystemPath;
    ///当前项目的项目数据
    QJsonObject _currentProjectInfo;
    ///当前项目的所有flowScene数据
    QStringList _currentFlowSceneSaveFiles;
    ///最近访问的项目 QMap的key为项目名称，value为项目的路径
    QList<QMap<QString,QString>> _recentProjects;


public:
    const QString &currentProjectName() const{return _currentProjectName;}
    void setCurrentProjectName(const QString &newCurrentProjectName){_currentProjectName = newCurrentProjectName;}
    const QString &currentProjectPath() const{return _currentProjectPath;}
    void setCurrentProjectPath(const QString &newCurrentProjectPath){
        _currentProjectPath = newCurrentProjectPath;
        _currentProjectSubSystemPath = newCurrentProjectPath+"/subsystem";
    }
    const QString &currentProjectSubSystemPath() const{return _currentProjectSubSystemPath;}
    const QJsonObject &currentProjectInfo() const{return _currentProjectInfo;}
    void setCurrentProjectInfo(const QJsonObject &newCurrentProjectInfo){_currentProjectInfo = newCurrentProjectInfo;}
    const QStringList &currentFlowSceneSaveFiles() {return _currentFlowSceneSaveFiles;}
    void setCurrentFlowSceneSaveFiles(const QStringList &newCurrentFlowSceneSaveFiles){_currentFlowSceneSaveFiles = newCurrentFlowSceneSaveFiles;}
    const QList<QMap<QString, QString> > &recentProjects() const{return _recentProjects;}
    void setRecentProjects(const QList<QMap<QString, QString> > &newRecentProjects){_recentProjects = newRecentProjects;}
    void addRecentProject( QString pname, QString ppath){
        QMap<QString,QString> map;
        map.insert(pname,ppath);
        _recentProjects.append(map);
        emit addRecentProjectEvent(pname,ppath);
    };


    void readProjectXml(QFile &file){
        QDomDocument doc;

        if(!doc.setContent(&file)){
            file.close();
            return;
        }

        //获取项目名称
        QDomElement root = doc.documentElement();
        _currentProjectName = root.attributeNode("name").value();

        //获取项目路径
        QStringList qsl = file.fileName().split("/"+_currentProjectName+"/.ap");
        _currentProjectPath=qsl[0];

        if(_currentProjectName==""){
            QMessageBox::critical(Q_NULLPTR,"危险","项目文件出错，请重新选择项目文件",QMessageBox::Ok,QMessageBox::Ok);
    //        QMessageBox::critical(Q_NULLPTR,"critical","项目文件出错，请重新选择项目文件",QMessageBox::Ok,QMessageBox::Ok);
            return;
        }

        QDomNodeList flowScenes = root.childNodes().item(0).childNodes();
        for(int i=0;i<flowScenes.count();i++){
            QDomNode flowScene = flowScenes.item(i);
            _currentFlowSceneSaveFiles.append(flowScene.toElement().attributeNode("saveFile").value());
        }

    }
};


#endif // MODELS_PROJECT_H
