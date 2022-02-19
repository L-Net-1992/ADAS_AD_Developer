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
#include <dialogs/nodetreedialog.h>
#include <dialogs/testdialog.h>
#include <dialogs/datainspectordialog.h>
#include <dialogs/editordialog.h>
#include <dialogs/calibrationdialog.h>
#include "stdio.h"
#include "utils.h"
#include "controllers/aicctreewidget.hpp"
#include "controllers/aicctoolbutton.hpp"
#include "nodeparser/source_generator.hpp"
#include "nodeparser/models.hpp"
#include "sqlite/aiccsqlite.hpp"


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
//    void

private:
    Ui::MainWindow *ui;
    AICCSqlite sqlite;
    ProjectDialog *projectDialog;
    NodeParametersDialog *npDialog;
    NodeTreeDialog *nodeTreeDialog;
    ImportScriptDialog *isDialog;
    DataInspectorDialog *diDialog;
    EditorDialog *eDialog;
    CalibrationDialog *cDialog;
    QSharedPointer<ModuleLibrary> _moduleLibrary;                                                            //脚本导入node的模型数据
    QProcess * process;

    //nodeeditor部分
    QMap<QString,QSet<QString>> nodeMap;

    AICCStackedWidget asw;

    void initMenu();
    void initTreeView();

    void fillTableData(QTableWidget *tw,const NodeDataModel *nmd);

    void initSplitter();
    void initTableWidget();
    void initToolbar();
    void setTreeNode(QTreeWidget *tw,const char* ptext,const char* picon);
    void initBreadcrumbNavigation();
    void initStackedWidget();
    void initImportScriptDialog();
    void initProjectDialog();
    void initDataInspectorDialog();

    //动作函数部分
    void pbOpenAction();

    /* xlguo debug*/
    std::string modules_path_ = "/home/guopeng/code/liudian/adas_node/output/"; // /home/guopeng/code/xlguo/install
    std::string common_library_path_ = "/home/guopeng/code/liudian/adas_node/output/"; 

private:
    void initNodeEditor();
    std::shared_ptr<DataModelRegistry> registerDataModels(const std::list<Invocable> parserResult);
    QMap<QString,QSet<QString>> nodeCategoryDataModels(const std::list<Invocable> parseResult);
    static void logOutput(QtMsgType type,const QMessageLogContext &context,const QString &msg);


private:
//    static QString directMsg;
    static void write(QString str);

public:
Q_SIGNALS:
     void redirectMsg(QString text);

public:
    static inline QPlainTextEdit *pte_out = Q_NULLPTR;

};


#endif // MAINWINDOW_H
