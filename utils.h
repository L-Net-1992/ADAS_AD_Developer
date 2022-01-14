#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QDir>
#include <conf/Config.hpp>
#include <QApplication>

 QStringList getFlieList(const QString &path);

 QJsonObject getConfig();

 QList<QPair<QString,QJsonObject>> orderedQJsonObject(QJsonObject jo);
#endif // UTILS_H
