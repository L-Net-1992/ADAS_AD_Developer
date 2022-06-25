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

/**
 * @brief recursionQJsonObjectModuleBrowser 递归2级子节点并带入父分类信息
 * @param parentName
 * @param jo
 * @param twi
 */
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

/**
 * @brief getParentidByPath     通过字符串形式的路径获得最下一层的parentid
 * @param path                  字符串形式路径由|分隔每层路径
 * @param pid                   未查到该路径对应的parentid返回pid默认值
 * @return                      返回最下一层路径的parentid
 */
int getParentidByPath(std::string path,const int pid){
    int rpid = pid;
    std::vector<std::string> v_split;
    boost::split(v_split,path,boost::is_any_of("|"),boost::token_compress_on);
    if(v_split.size()==0) return rpid;

    std::string pname = v_split.at(0);
//    qDebug() << "-----------------------pname:" << QString::fromStdString(pname);
    AICCSqlite sqlite;
    QString sql = QString("select id from modelNode where caption = '%0' and is_node = 0 and parentid = %1").arg(QString::fromStdString(pname)).arg(pid);
    QSqlQuery squery = sqlite.query(sql);
    if(squery.next()){
        rpid = squery.value(0).toInt();
        boost::erase_first(path,(pname.append("|")));
        return getParentidByPath(path,rpid);
    }else
        return rpid;
}

/**
 * @brief classNameIsExist      判断class_name在数据库中是否存在
 * @param class_name
 * @return
 */
bool classNameIsExist(std::string class_name){
    AICCSqlite sqlite;
    QString sql = QString("select count(*) from modelNode where class_name = '%0'").arg(QString::fromStdString(class_name));
    QSqlQuery query = sqlite.query(sql);
    if(query.next()){
        if(query.value(0).toInt()>0) return true;
        else return false;
    }
    else{
        return false;
    }
}

/**
 * @brief addSubsystem          增加一条新的node数据进去
 * @param pid                   父id
 * @param package               包名
 * @param class_name            包名+类名
 * @param caption               标题
 * @return
 */
bool addSubsystem2DB(const int pid,const std::string package,const std::string class_name,const std::string caption){
    if(!classNameIsExist(class_name)){
        AICCSqlite sqlite;
        QSqlQuery query = sqlite.sqlQuery();
//        QString sql = QString("insert into modelNode ('parentid','class_name','caption','is_node') values (%0,'%1','%2',1)").arg(pid).arg(QString::fromStdString(class_name)).arg(QString::fromStdString(caption));
        query.prepare("insert into modelNode (parentid,class_name,caption,is_node) values (?,?,?,?)");
        query.addBindValue(pid);
        query.addBindValue(QString::fromStdString(class_name));
        query.addBindValue(QString::fromStdString(caption));
        query.addBindValue(1);
        bool ret = query.exec();
        return ret;
    }
    return false;
}

/**
 * @brief importWithoutModuleSubsystem  检查所有得invocable，如果不存在数据库中添加
 * @param invocableVector               当前系统中导入得所有invocable
 * @return                              返回导入invocable
 */
QVector<QString> importWithoutModuleSubsystem(std::vector<Invocable> invocableVector){
    QVector<QString> importClassName;

    QString sql = "select count(*) as c from modelNode where class_name = ?";
    for(int i=0;i<invocableVector.size();i++){
        Invocable invocable = invocableVector.at(i);
        std::string package = invocable.getPackage();
        std::string className = invocable.getName();
//                qDebug() << "------------------package:" << QString::fromStdString(package) << " | name:" << QString::fromStdString(name);
//        QString className = QString::fromStdString(name);
        AICCSqlite sqlite;
        QSqlQuery query = sqlite.sqlQuery();
        query.prepare(sql);
        query.addBindValue(QString::fromStdString(className));
        if(query.exec() && query.next()){
            if(query.value("c").toInt()>=1) {

                continue;
            }
            else{
                qInfo() << "className:" << QString::fromStdString(className) << "是新模块,增加到数据库中";
                //pid固定设置为3，分类是自定义模块
                if(addSubsystem2DB(3,package,className,className)){
                    importClassName.append(QString::fromStdString(className));
                }
            }
        }

    }

    return importClassName;
}



/**
 * @brief copyDirectory     拷贝文件夹
 * @param srcPath
 * @param dstPath
 * @param coverFileIfExist
 * @return
 */
bool copyDirectory(const QString& srcPath, const QString& dstPath, bool coverFileIfExist)
{
    QDir srcDir(srcPath);
    QDir dstDir(dstPath);
    if (!dstDir.exists()) { //目的文件目录不存在则创建文件目录
        if (!dstDir.mkdir(dstDir.absolutePath()))
            return false;
    }
    QFileInfoList fileInfoList = srcDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir()) {    // 当为目录时，递归的进行copy
            if (!copyDirectory(fileInfo.filePath(),dstDir.filePath(fileInfo.fileName()),coverFileIfExist))
                return false;
        }
        else {            //当允许覆盖操作时，将旧文件进行删除操作
            if (coverFileIfExist && dstDir.exists(fileInfo.fileName())) {
                dstDir.remove(fileInfo.fileName());
            }
            /// 进行文件copy
            if (!QFile::copy(fileInfo.filePath(), dstDir.filePath(fileInfo.fileName()))) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief copyFile          拷贝文件
 * @param srcPath
 * @param dstPath
 * @param coverFileIfExist
 * @return
 */
bool copyFile(QString srcPath, QString dstPath, bool coverFileIfExist)
{
    srcPath.replace("\\", "/");
    dstPath.replace("\\", "/");
    if (srcPath == dstPath) {
        return true;
    }

    if (!QFile::exists(srcPath)) {  //源文件不存在
        return false;
    }

    if (QFile::exists(dstPath)) {
        if (coverFileIfExist) {
            QFile::remove(dstPath);
        }
    }

    if (!QFile::copy(srcPath, dstPath)){
        return false;
    }
    return true;
}

/**
 * @brief getCaptionByName  通过模块名称在数据库中查找caption信息
 * @param name              模块名称
 * @return                  返回caption信息
 */
QString getCaptionByName(QString name){
    QString caption = name;
    AICCSqlite sqlite;
    const QString sql = QString("select caption from modelNode where class_name = '%0'").arg(name);
    QSqlQuery query = sqlite.query(sql);
    if(query.next())
        caption = query.value("caption").toString();
    return caption;
}
