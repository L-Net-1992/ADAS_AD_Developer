#include "utils.h"

QStringList getFlieList(const QString &path){
    QDir dir(path);
    QStringList nameFilters;
    nameFilters << "*.hpp";
    //    QStringList files = dir.entryList(nameFilters,QDir::Files|QDir::Readable,QDir::Name);
    //    QFileInfoList filist =  dir.entryInfoList(nameFilters,QDir::Readable,QDir::Name);
    QFileInfoList filist = dir.entryInfoList(nameFilters);
    QStringList files;
    for(QFileInfo fileInfo:filist)
        files << fileInfo.absoluteFilePath();
    return files;
}

QJsonObject getConfig(){
    QJsonObject ret;
    Config config(QApplication::applicationDirPath()+"/conf/config.json");
#ifdef Q_OS_WIN64
    ret = config.readJsonObject("win64Path");
#endif
#ifdef Q_OS_LINUX
    ret = config.readJsonObject("ubuntuPath");
#endif
    return ret;
}

QJsonObject getUbuntuConfig(const QString &rootpath){
    Config config(rootpath+"/conf/config.json");
    QJsonObject ret = config.readJsonObject("ubuntuPath");
    return ret;
}

QJsonObject getWin64Config(const QString &rootpath){
    Config config(rootpath+"/conf/config.json");
    QJsonObject ret = config.readJsonObject("win64Path");
    return ret;
}
