//
// Created by 冯冲 on 2022/4/6.
//

#ifndef AICCPROCESS_H
#define AICCPROCESS_H

#include <QtCore/QObject>
#include <QApplication>
#include <QVector>
#include <QProcess>
#include <QQueue>
#include <QDebug>
#include <QWidget>

class AICCProcess:public QProcess{
    Q_OBJECT
public:
    explicit AICCProcess(QVector<QWidget*> vw):_vwidgets(vw){

        connect(this,static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),[this](int exitCode,QProcess::ExitStatus exitStatus){
            setAllEnabledWidget(true);
            if(exitCode){
                qInfo() << "process finished:脚本执行完成";
            }else{
                qInfo() << "process finished:脚本执行出错，终止任务。错误码:" << exitCode;
                return;
            }

            queueProcessStart();
        });
    }
    ~AICCProcess(){}

    void processStart(const QVector<QString> scriptNames,const int platformIndex){
        QString bash="bash ";
        bash.append(QApplication::applicationDirPath()).append("/App/").append(scriptNames[platformIndex-1]);
        QString killprocess = "kill -9 $(ps -ef|gre        if(series_group_!=Q_NULLPTR)p adas_generate|grep -v grep|awk '{print $2}')";
        AICCProcess::terminate();
        AICCProcess::start(bash);
    }

    void mulProcessStart(QVector<QString> scriptNames){
        if(_taskQueue.size()>0){
            qInfo() << "上个队列任务进行中，请稍后增加新任务";
            return;
        }

        QVector<QString>::iterator it;
        for(it = scriptNames.begin();it != scriptNames.end();it++){
            _taskQueue.enqueue(*it);
        }

        queueProcessStart();
    }

    /**
     * @brief addSetEnabledWidget   增加需要屏蔽得Widget控件
     * @param w
     */
    void addSetEnabledWidget(QWidget *w){
        _vwidgets.append(w);
    }
    void setVwidgets(const QVector<QWidget *> &newVwidgets)
    {
        _vwidgets = newVwidgets;
    }


private:
    void queueProcessStart(){
        if(_taskQueue.size()==0){
            qInfo() << "队列任务为空";
            return;
        }

        setAllEnabledWidget(false);

        QString bash = "bash ";
        QString taskScript = _taskQueue.dequeue();
        qInfo() << "执行脚本:" << taskScript;
        bash.append(QApplication::applicationDirPath()).append("/App/").append(taskScript);
        AICCProcess::terminate();
        AICCProcess::start(bash);



    }

    /**
     * @brief setAllEnabledWidget   设置所有监控得控件可用或不可用
     * @param f
     */
    void setAllEnabledWidget(bool f){
        QVector<QWidget*>::iterator it;
        for(it = _vwidgets.begin();it!=_vwidgets.end();it++)
            (*it)->setEnabled(f);
    }

private:
    QQueue<QString> _taskQueue;
    QVector<QWidget*> _vwidgets;                    //当process执行时需要不可用得按钮
};

#endif // AICCPROCESS_H
