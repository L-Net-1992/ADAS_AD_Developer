#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTableWidget>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QtGlobal>
#include <itemreadonlydelegate.h>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QDrag>
#include <QGraphicsView>
#include <QString>
#include <QStackedLayout>
#include <QSharedPointer>
#include <QtConcurrent>
#include <QMetaType>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <fstream>

#include <nodes/FlowScene>
#include <nodes/FlowView>
#include <nodes/FlowViewStyle>
#include <nodes/NodeStyle>
#include <nodes/ConnectionStyle>
#include <nodes/DataModelRegistry>
#include <controllers/aiccflowscene.hpp>
#include <controllers/aiccflowview.hpp>
#include <controllers/aiccstackedwidget.hpp>
#include <controllers/aiccstructtreewidget.hpp>
#include <dialogs/projectdialog.h>
#include <dialogs/nodeparametersdialog.h>
#include <dialogs/nodeparametersmildialog.h>
#include <dialogs/nodetreedialog.h>
#include <dialogs/testdialog.h>
#include <dialogs/datainspectordialog.h>
#include <dialogs/monitordialog.h>
#include <dialogs/editorwindow.h>
#include <dialogs/calibrationdialog.h>
#include <dialogs/recentprojectdialog.h>
#include <dialogs/exportmoduledialog.h>
#include <dialogs/new_subsystem_dialog.h>

#include "dialogs/subsystem_window.h"
#include "dialogs/new_subsystem_dialog.h"
#include "stdio.h"
#include "utils.h"
#include "controllers/aicctreewidget.hpp"
#include "controllers/aicctoolbutton.hpp"
#include "ADAS_AD_Backend/source_generator.hpp"
#include "ADAS_AD_Backend/models.hpp"
#include "ADAS_AD_Backend/subsystem_library.h"

#include "sqlite/aiccsqlite.hpp"
#include "model/modelsproject.hpp"
#include "model/modelcategory.hpp"
#include "process/aiccprocess.hpp"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;
using QtNodes::FlowViewStyle;
using QtNodes::NodeStyle;
using QtNodes::ConnectionStyle;




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void registrySceneGenerateNodeMenu(std::list<Invocable> parserResult);
    void closeEvent(QCloseEvent *e);
Q_SIGNALS:
    void scriptParserCompleted(std::list<Invocable> parserResult);

private:
    void initProcess();
    void initMenu();
    void initTreeView();

    void fillTableData(QTableWidget *tw,const NodeDataModel *nmd);

    void initSplitter();
    void initTableWidget();
    void refreshTreeViewDynamicNode();                                       //刷新动态节点
    void initToolbar();
    void initBreadcrumbNavigation();
    void initStackedWidget();
    void initImportScriptDialog();
    void initRecentProjectDialog();
    void initDataInspectorDialog();
    void initProjectDialog();
    void initProjectDataModel();

    void saveProjectAction();
    bool openProjectAction();
    //打开项目动作函数部分
    void projectDataModelLoadCompletedAction(const QString pname,const QString ppath);
    //创建子系统动作函数
//    void createSubsysetmAction();

    void initNodeEditor();
    //生成代码
    void generateCode();
    void scriptParserCompletedAction(std::list<Invocable> parserResult);
    std::shared_ptr<DataModelRegistry> registerDataModels();
    QMap<QString,QSet<QString>> nodeCategoryDataModels(const std::list<Invocable> parseResult);

    void importCompletedAction();
    void sceneLoadFromMemoryCompletedAction(bool isCompleted);
    void refreshLeftTreeData();
    void invocableParserAction(const std::string msg);
    void openSubsystem(const std::string package,const std::string name);
public:
    void showMsg(QString msg);

private:
    Ui::MainWindow *ui;
//    AICCSqlite sqlite;
    ProjectDialog *projectDialog;
    RecentProjectDialog *rProjectDialog;
    NodeParametersDialog *npDialog;
    NodeParametersMILDialog *npmilDialog;
    NodeTreeDialog *nodeTreeDialog;
    ImportScriptDialog *isDialog;
    DataInspectorDialog *diDialog;
    MonitorDialog *monitorDialog;
    EditorWindow *eDialog;
    CalibrationDialog *cDialog;
    ExportModuleDialog *emDialog;
    QSharedPointer<ModuleLibrary> _moduleLibrary;                                                            //脚本导入node的模型数据
    //QSharedPointer<SubsystemLibrary> _subsystemLibrary;
    QSharedPointer<AICCProcess> _process;

    //nodeeditor部分
    QMap<QString,QSet<QString>> nodeMap;
    AICCStackedWidget asw;

    //project部分

    QSharedPointer<ProjectDataModel> _currentProjectDataModel;
    QSharedPointer<RecentProjectDataModel> _recentProjectDataModel;
    QSharedPointer<CategoryDataModel> _categoryDataModel;

    bool forceClose = false;
};


#endif // MAINWINDOW_H
