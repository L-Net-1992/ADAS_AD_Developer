#include "utils.h"

QStringList getFileList(const QString &path){
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

///只加载adas_packages.json一个文件
QStringList getADASPackagesFileList(const QString &path){
    QDir dir(path);
    QStringList nameFilters;
    nameFilters << "adas-packages.json";
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

///对 json数据进行排序
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


///主界面左侧功能树递归 child node
void recursionQJsonObjectLeft(QString parentName,QJsonObject jo,QTreeWidgetItem *twi){
    QStringList keys = jo.keys();
    for(int i=0;i<keys.size();i++){
        if(keys[i]=="order") continue;
        QJsonValue jv = jo.value(keys[i]);
        QTreeWidgetItem *ctwi = new QTreeWidgetItem(twi);
        ctwi->setText(0,keys[i]);
        QString path = parentName+"|"+keys[i];
        if(jv.isObject())
            recursionQJsonObjectLeft(path,jv.toObject(),ctwi);
        else if(jv.isArray())
            makeLeafNode(path,jv.toArray(),ctwi);
    }
}

//递归2及子节点并带入父分类信息
void recursionQJsonObjectModuleBrowser(QString parentName,QJsonObject jo,QTreeWidgetItem *twi){
    QStringList keys = jo.keys();
    for(int i=0;i<keys.size();i++){
        if(keys[i]=="order") continue;
        QJsonValue jv = jo.value(keys[i]);
        QTreeWidgetItem *ctwi = new QTreeWidgetItem(twi);
        ctwi->setText(0,keys[i]);
        QString path = parentName+"|"+keys[i];
        ctwi->setData(0,Qt::UserRole+1,QVariant::fromValue(path));
        //设置完数据后继续递归
        if(jv.isObject())
            recursionQJsonObjectModuleBrowser(path,jv.toObject(),ctwi);
        else if(jv.isArray())
            continue;
    }
}


///创建功能的叶子节点
void makeLeafNode(const QString path,QJsonArray ja,QTreeWidgetItem *twi){
    AICCSqlite sqlite;
    for(int i=0;i<ja.size();i++){
        QTreeWidgetItem *ctwi = new QTreeWidgetItem(twi);
        QString name = "";
        const QString caption = ja[i].toString();
        ctwi->setText(0,caption);
        //检索数据库中对应名称的node模块
        QString sql = QString("select n.id,n.name,n.caption from node n inner join nodeClass nc on n.class_id = nc.id where n.caption = '%0' and nc.class_name = '%1'").arg(caption,path);
        QSqlQuery squery = sqlite.query(sql);
        if(squery.next()){
            name = squery.value(1).toString();
        }else{
            name="";
        }
        ctwi->setData(0,Qt::UserRole+1,QVariant::fromValue(name));

        QIcon icon;
        icon.addPixmap(QPixmap("/res/function.png"));
        ctwi->setIcon(0,icon);
    }
}
