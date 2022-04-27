#ifndef RECENTPROJECTDIALOG_H
#define RECENTPROJECTDIALOG_H

#include <QDialog>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QListWidgetItem>
#include <model/modelsrecentproject.hpp>
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
    RecentProjectDialog(QSharedPointer<RecentProjectDataModel>rpdm,QWidget *parent = nullptr);
    ~RecentProjectDialog();

Q_SIGNALS:
    void setCurrentProjectDataModelCompleted(QSharedPointer<ProjectDataModel>pdm);
    void recentProjectDialogClosed();
    void newProjectTriggered();


private:
    void initStyle();
    void initPlatformConfig();
    void setDoubleClickAction();
    void initObserver();
    void initConnect();
    void openProjectAction(QListWidgetItem *item);
private:
    Ui::RecentProjectDialog *ui;
    QSharedPointer<RecentProjectDataModel> _recent_project_data_model;
    QSharedPointer<ProjectDataModel> _project_data_model;
};

#endif // RECENTPROJECTDIALOG_H
