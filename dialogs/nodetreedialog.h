#ifndef NODETREEDIALOG_H
#define NODETREEDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolButton>
#include <QPushButton>
#include <QSharedPointer>
#include <QString>
#include <QTreeWidgetItem>
#include <QSqlQuery>
#include <QJsonArray>
#include <QMap>
#include <QVariant>
#include <iostream>
#include <list>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "stdio.h"
#include "controllers/aicctoolbutton.hpp"
#include "controllers/aiccstackedwidget.hpp"
#include "controllers/aicctreewidget.hpp"
#include "sqlite/aiccsqlite.hpp"
#include "utils.h"
#include <dialogs/importscriptdialog.h>


#include <nodes/DataModelRegistry>

//#include <nodeparser/invocable.hpp>
//#include <nodeparser/invocable_parser.hpp>
//#include <nodeparser/models.hpp>
//#include <nodeparser/module_library.hpp>
//#include <nodeparser/new_subsystem_dialog.h>
#include <ADAS_AD_Backend/invocable.hpp>
#include <ADAS_AD_Backend/invocable_parser.hpp>
#include <ADAS_AD_Backend/models.hpp>
#include <ADAS_AD_Backend/module_library.hpp>
#include "dialogs/new_subsystem_dialog.h"


#include <model/modelcategory.hpp>


using QtNodes::DataModelRegistry;

//AICCToolButton * createToolButton( QString name);

namespace Ui {class NodeTreeDialog;}

class NodeTreeDialog : public QDialog
{
    Q_OBJECT

public:
    using CategoriesSet = std::set<QString>;

//    explicit NodeTreeDialog(QWidget *parent = Q_NULLPTR);
    NodeTreeDialog(QSharedPointer<CategoryDataModel> cdm,QSharedPointer<ProjectDataModel> pdm,QWidget *parent = Q_NULLPTR);
//    explicit NodeTreeDialog(QWidget *parent = Q_NULLPTR,ImportScriptDialog *isd=Q_NULLPTR);
    ~NodeTreeDialog();
    void setNodeMap(QMap<QString,QSet<QString>> pnm);

    void setModuleLibrary(ModuleLibrary *newModuleLibrary);

    void setSubsystemLibrary(SubsystemLibrary *newSubsystemLibrary);

signals:
    void nodeDoubleClicked(const QString nodeName);

private slots:
    void itemClickedAction(QTreeWidgetItem *item,int column);

    void modelSearchUpdate();

private:
    void initTreeWidget();
    void initToolBar();
    void initNodeButtonTable();
    void makeRootGroupItem(AICCTreeWidget *atw,const QString name,const QString text);
    void makeModelMenuItem(AICCTreeWidget *atw);
    AICCToolButton *createToolButton(QString id,QString parentid, QString name,QString caption,QString iconName);

private:
    void initNodeTree(AICCTreeWidget * tw_root,const QJsonObject json);
    void recursionChildren(QJsonObject json,QTreeWidgetItem *twp,int pid);


private:
    Ui::NodeTreeDialog *ui;
    QSharedPointer<CategoryDataModel> _categoryDataModel;
    QSharedPointer<ProjectDataModel> _projectDataModel;
//    ModuleLibrary *_moduleLibrary;
//    SubsystemLibrary *_subsystemLibrary;


};

#endif // NODETREEDIALOG_H
