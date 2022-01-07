#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QDir>
#include <conf/Config.hpp>
#include <QApplication>

 QStringList getFlieList(const QString &path);

 QJsonObject getConfig();
 QJsonObject getUbuntuConfig(const QString &rootpath);
 QJsonObject getWin64Config(const QString &rootpath);
#endif // UTILS_H
