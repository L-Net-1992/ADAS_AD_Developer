#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QDir>
#include <conf/Config.hpp>
#include <QApplication>
#include <QTreeWidgetItem>
#include <QJsonArray>
#include "sqlite/aiccsqlite.hpp"
#include "boost/algorithm/string.hpp"
//#include "boost/algorithm/string/find.hpp"

 QStringList getFileList(const QString &path);
 QStringList getADASPackagesFileList(const QString &path);

 QJsonObject getConfig();

 QList<QPair<QString,QJsonObject>> orderedQJsonObject(QJsonObject jo);

 void makeLeafNode(QString path,QJsonArray ja,QTreeWidgetItem *twi);

 void recursionQJsonObjectLeft(QString parentName,QJsonObject jo,QTreeWidgetItem *twi);
 void recursionQJsonObjectModuleBrowser(QString parentName,QJsonObject jo,QTreeWidgetItem *twi);

 bool copyDirectory(const QString& srcPath, const QString& dstPath, bool coverFileIfExist);
 bool copyFile(QString srcPath, QString dstPath, bool coverFileIfExist);
 QString getCaptionByName(QString name);
 int getParentidByPath(std::string path,const int pid);
 bool classNameIsExist(std::string class_name);
 bool addSubsystem(const int pid,const std::string package,const std::string name,const std::string caption);
#endif // UTILS_H
