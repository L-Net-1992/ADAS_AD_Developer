#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QDir>
#include <conf/Config.hpp>
#include <QApplication>
#include <QTreeWidgetItem>
#include <QJsonArray>
#include "sqlite/aiccsqlite.hpp"

 QStringList getFileList(const QString &path);
 QStringList getADASPackagesFileList(const QString &path);

 QJsonObject getConfig();

 QList<QPair<QString,QJsonObject>> orderedQJsonObject(QJsonObject jo);

 void makeLeafNode(QJsonArray ja,QTreeWidgetItem *twi);

 void recursionQJsonObject(QJsonObject jo,QTreeWidgetItem *twi);
#endif // UTILS_H
