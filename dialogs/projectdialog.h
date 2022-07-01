#ifndef PROJECTDIALOG_H
#define PROJECTDIALOG_H

#include <QDialog>
#include <iostream>
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QtXml/QDomDocument>
#include <cmath>
#include <model/modelsproject.hpp>
#include <model/modelsrecentproject.hpp>

namespace Ui {
class ProjectDialog;
}

using namespace std;

class ProjectDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectDialog(QSharedPointer<ProjectDataModel> pdm,QSharedPointer<RecentProjectDataModel> rpdm,QWidget *parent);
    ~ProjectDialog();

    const QString &getProjectName() const;
    const QString &getProjectPath() const;
    const QStringList &getFlowSceneSaveFiles() const;
    void readProjectXml(QFile &file);
    void setProjectPath(const QString &newProjectPath);
    void showFromRecentProjectDialog();

Q_SIGNALS:
    void projectDialogCanceled(bool showRecentProjectDialog);

private:
    void initButton();
    void initOpenPath();
    void initStackedFirst();
    void initStackedSecond();
    void writeProjectXml(QFile &file);

private:
    Ui::ProjectDialog *ui;
    QString projectName;
    //项目路径保存到项目名的上一层，/home/fc/works/AutoCar
    QString projectPath;
    QString config1;
    QString config2;
    QStringList flowSceneSaveFiles;
    QSharedPointer<ProjectDataModel> pDataModel;
    QSharedPointer<RecentProjectDataModel> rProjectDataModel;
    bool rebackRecentProject;
};

#endif // PROJECTDIALOG_H
