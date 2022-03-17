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
#include <controllers/aiccflowview.hpp>
#include <controllers/aiccstackedwidget.hpp>
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
#include "stdio.h"
#include "utils.h"
#include "controllers/aicctreewidget.hpp"
#include "controllers/aicctoolbutton.hpp"
#include "nodeparser/source_generator.hpp"
#include "nodeparser/models.hpp"
#include "nodeparser/subsystem_library.h"
#include "sqlite/aiccsqlite.hpp"
#include "project/modelsproject.hpp"


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
Q_SIGNALS:
    void scriptParserCompleted(std::list<Invocable> parserResult);

private:


    void initMenu();
    void initTreeView();

    void fillTableData(QTableWidget *tw,const NodeDataModel *nmd);

    void initSplitter();
    void initTableWidget();
    void refreshTreeViewDynamicNode();                                       //刷新动态节点
    void initToolbar();
    void setTreeNode(QTreeWidget *tw,const char* ptext,const char* picon);
    void initBreadcrumbNavigation();
    void initStackedWidget();
    void initImportScriptDialog();
    void initProjectDialog();
    void initRecentProjectDialog();
    void initDataInspectorDialog();

    //打开项目动作函数部分
    void pbOpenAction(QString projectPath = Q_NULLPTR);
    //创建子系统动作函数
    void createSubsysetmAction();

    void initNodeEditor();
    void scriptParserCompletedAction(std::list<Invocable> parserResult);
    std::shared_ptr<DataModelRegistry> registerDataModels();
//    QMap<QString,QSet<QString>> nodeCategoryDataModels(const std::list<Invocable> parseResult);
    QMap<QString,QSet<QString>> newNodeCategoryDataModels(const std::list<Invocable> parseResult);
    static void logOutput(QtMsgType type,const QMessageLogContext &context,const QString &msg);
    void processStart(const QVector<QString> scriptNames,const int platformIndex);


    static void write(QString str);

public:
Q_SIGNALS:
     void redirectMsg(QString text);

public:
    static inline QPlainTextEdit *pte_out = Q_NULLPTR;

private:
    Ui::MainWindow *ui;
    AICCSqlite sqlite;
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
//    QSharedPointer<ModuleLibrary> _moduleLibrary;                                                            //脚本导入node的模型数据
//    QSharedPointer<SubsystemLibrary> _subsystemLibrary;
    ModuleLibrary *_moduleLibrary;
    SubsystemLibrary *_subsystemLibrary;
    QProcess * process;

    //nodeeditor部分
    QMap<QString,QSet<QString>> nodeMap;
    AICCStackedWidget asw;

    //project部分
    ProjectDataModel *pDataModel;
};


#endif // MAINWINDOW_H
