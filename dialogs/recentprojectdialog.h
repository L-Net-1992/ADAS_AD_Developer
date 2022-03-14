#ifndef RECENTPROJECTDIALOG_H
#define RECENTPROJECTDIALOG_H

#include <QDialog>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QListWidgetItem>
#include <project/modelsproject.hpp>
#include <QDebug>
#include <QFont>
#include <QLabel>
#include <QDir>

namespace Ui {
class RecentProjectDialog;
}

class RecentProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecentProjectDialog( ProjectDataModel *pdm,QWidget *parent = nullptr);
    ~RecentProjectDialog();

Q_SIGNALS:
    void setCurrentProjectDataModelCompleted(ProjectDataModel *projectPath);
    void recentProjectDialogClosed();
    void newProjectTriggered();


private:
    void initPlatformConfig();
    void setDoubleClickAction(QListWidgetItem *li);
    void initObserver();
    void initConnect();
    void openProject(QListWidgetItem *item);
    bool existProject(const QString ppath);
private:
    Ui::RecentProjectDialog *ui;
    ProjectDataModel *_project_data_model;
};

#endif // RECENTPROJECTDIALOG_H
