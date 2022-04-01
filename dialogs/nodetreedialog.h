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

#include <nodeparser/invocable.hpp>
#include <nodeparser/invocable_parser.hpp>
#include <nodeparser/models.hpp>
#include <nodeparser/module_library.hpp>


using QtNodes::DataModelRegistry;

//AICCToolButton * createToolButton( QString name);

namespace Ui {
class NodeTreeDialog;
}

class NodeTreeDialog : public QDialog
{
    Q_OBJECT

public:
    using CategoriesSet = std::set<QString>;

    explicit NodeTreeDialog(QWidget *parent = Q_NULLPTR);
//    explicit NodeTreeDialog(QWidget *parent = Q_NULLPTR,ImportScriptDialog *isd=Q_NULLPTR);
    ~NodeTreeDialog();
    void setNodeMap(QMap<QString,QSet<QString>> pnm);

signals:
    void nodeDoubleClicked(const QString nodeName);

private slots:
    void itemClickedAction(QTreeWidgetItem *item,int column);

private:
    void initTreeWidget();
    void initToolBar();
    void initNodeButtonTable();
    void makeRootGroupItem(AICCTreeWidget *atw,const QString name,const QString text);
    void makeModelMenuItem(AICCTreeWidget *atw);
    AICCToolButton *createToolButton(QString id, QString name,QString caption,QString iconName);

private:
    void initNodeTree(AICCTreeWidget * tw_root);
    void recursionChildren(QTreeWidgetItem *twp,int pid);

private:
    Ui::NodeTreeDialog *ui;
    QMap<QString,QSet<QString>> _nodeMap;
    AICCSqlite _sqlite;

};

#endif // NODETREEDIALOG_H
