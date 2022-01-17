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

///order json
QList<QPair<QString,QJsonObject>> orderedQJsonObject(QJsonObject jo){
    QList<QPair<QString,QJsonObject>> list;
    QStringList keys = jo.keys();
//    for(QString key:jo.keys()){
    for(auto it = keys.begin();it!=keys.end();it++){
        QPair<QString,QJsonObject> pair;
        pair.first = QString::fromStdString(it->toStdString());
        pair.second = jo[pair.first].toObject();
        list.insert(pair.second["order"].toInt(), pair);
    }
    return list;
}


///recursion child node
void recursionQJsonObject(QJsonObject jo,QTreeWidgetItem *twi){
    QStringList keys = jo.keys();
//    for(auto key:keys){
    for(int i=0;i<keys.size();i++){
        if(keys[i]=="order") continue;
        QJsonValue jv = jo.value(keys[i]);
        QTreeWidgetItem *ctwi = new QTreeWidgetItem(twi);
        ctwi->setText(0,keys[i]);
        if(jv.isObject())
            recursionQJsonObject(jv.toObject(),ctwi);
        else if(jv.isArray())
            makeLeafNode(jv.toArray(),ctwi);
    }
}

void makeLeafNode(QJsonArray ja,QTreeWidgetItem *twi){
    for(int i=0;i<ja.size();i++){
        QTreeWidgetItem *ctwi = new QTreeWidgetItem(twi);
        ctwi->setText(0,ja[i].toString());
    }
}
