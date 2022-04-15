//
// Created by 冯冲 on 2022/4/6.
//

#ifndef AICCPROCESS_H
#define AICCPROCESS_H

#include <QtCore/QObject>
#include <QApplication>
#include <QVector>
#include <QProcess>

class AICCProcess:public QProcess{
    Q_OBJECT
public:
    AICCProcess(){}
    ~AICCProcess(){}

    void processStart(const QVector<QString> scriptNames,const int platformIndex){
        QString bash="bash ";
        bash.append(QApplication::applicationDirPath()).append("/App/").append(scriptNames[platformIndex-1]);
        QString killprocess = "kill -9 $(ps -ef|gre        if(series_group_!=Q_NULLPTR)p adas_generate|grep -v grep|awk '{print $2}')";
        this->terminate();
        this->start(bash);
    }
};

#endif // AICCPROCESS_H
