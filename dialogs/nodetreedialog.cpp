#include "nodetreedialog.h"
#include "ui_nodetreedialog.h"

NodeTreeDialog::NodeTreeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeTreeDialog)
{
    ui->setupUi(this);
    _sqlite.initDatabaseConnection();

    this->initTreeWidget();
    this->initToolBar();
    this->initNodeButtonTable();
}

NodeTreeDialog::~NodeTreeDialog()
{
    _sqlite.closeConnection();
    delete ui;
}
///初始化属性结构内容
void NodeTreeDialog::initTreeWidget()
{
    AICCTreeWidget *tw = ui->tw_nodeTree;
    tw->clear();

    //旧树形菜单
    //initToolBar();
    //    makeModelMenuItem(tw);
    //    connect(ui->tw_nodeTree,&AICCTreeWidget::itemClicked,this,&NodeTreeDialog::treeWidgetItemClicked);


    //可随意拖放菜单
    //    initNodeTree(tw);
    //    AICCSqlite sqlite;

    initNodeTree(tw);

    tw->expandAll();

    connect(tw,&QTreeWidget::itemClicked,this,&NodeTreeDialog::itemClickedAction);
}

///点击目录展示目录中所有模块
void NodeTreeDialog::itemClickedAction(QTreeWidgetItem *item,int column){
    qDebug() << "item click:" << item->text(0);
    this->setFixedWidth(1140);
    this->setFixedHeight(624);

    //清空表格内容，删除已有的按钮
    ui->tw_nodeModels->setRowCount(0);
    ui->tw_nodeModels->clearContents();

    //获得当前目录的id作为查询叶子节点的父id
    QVariant data = item->data(0,Qt::UserRole+1);
    int pid = data.toJsonObject().value("id").toInt();

    //增加点击分类对应的按钮
    //最多6列，根据按钮的总量决定行数
    int columnCount = 6;
    QString itemData = item->data(0,Qt::UserRole+1).value<QString>();

    //获得按钮相关数据
    QVariant v = item->data(0,Qt::UserRole+1);
    QJsonObject jo = v.toJsonObject();

    auto func_ptr = [](QSqlQuery q){
        QMap<QString,QVariant> v;
        v.insert("class_name",q.value("class_name").toString());
        v.insert("caption",q.value("caption").toString());
        v.insert("icon_name",q.value("icon_name").toString());
        return v;
    };
    AICCSqlite sl;
    //将结果集转换
    QString sql = QString("select class_name,caption,icon_name from modelNode where parentid = %0 and is_node=1").arg(pid);
    QVector<QMap<QString,QVariant>> vector =
            sl.query1(sql,func_ptr);


    ui->tw_nodeModels->setRowCount(vector.size()/8+1);


    QVectorIterator vit(vector);
    int i=0;
    while(vit.hasNext()){
        AICCToolButton *tb;

        const QMap<QString,QVariant> m = vit.next();
        QString className = m.value("class_name").toString();
        QString caption = m.value("caption").toString();
        QString iconName = m.value("icon_name").toString();
        tb = createToolButton(className,caption);
        tb->setNodeIcon(iconName.replace(":",""));
        ui->tw_nodeModels->setCellWidget(i/columnCount,i%columnCount,tb);
        i++;
    }
}

///左侧属性菜单部分_sqlite
void NodeTreeDialog::initNodeTree(AICCTreeWidget * tw_root){
    //查询所有根目录节点
    QVector<QMap<QString,QVariant>> vector = _sqlite.query1("select id,parentid,class_name,caption,is_node,icon_name from modelNode where parentid = 0",
                                                       [](QSqlQuery q){
            QMap<QString,QVariant> v;
            v.insert("id",q.value("id").toInt());
            v.insert("parentid",q.value("parentid").toInt());
            v.insert("caption",q.value("caption").toString());
            v.insert("icon_name",q.value("icon_name").toString());
            return v;

    });

    QVectorIterator vit(vector);
    while(vit.hasNext()){
        const QMap<QString,QVariant> m = vit.next();
        int id = m.value("id").toInt();
        QString caption = m.value("caption").toString();
        QString iconName = m.value("icon_name").toString();

        QTreeWidgetItem *twi = new QTreeWidgetItem(tw_root);
        twi->setText(0,caption);

        QIcon icon;
        icon.addPixmap(QPixmap(iconName));
        twi->setIcon(0,icon);

        QJsonObject jo;
        jo.insert("id",id);
        jo.insert("caption",caption);
        twi->setData(0,Qt::UserRole+1,QVariant::fromValue(jo));

        recursionChildren(twi,id);
    }

}

///递归所有子级节点
void NodeTreeDialog::recursionChildren(QTreeWidgetItem *twp,int pid){
    //查询pid下一级的子目录/节点
    QString sql = QString("select id,parentid,class_name,caption,is_node,icon_name from modelNode where parentid = %0").arg(pid);
    QVector<QMap<QString,QVariant>> vector = _sqlite.query1(sql,
                                                       [](QSqlQuery q){
            QMap<QString,QVariant> v;
            v.insert("id",q.value("id").toInt());
            v.insert("parentid",q.value("parentid").toInt());
            v.insert("caption",q.value("caption").toString());
            v.insert("is_node",q.value("is_node").toBool());
            v.insert("icon_name",q.value("icon_name").toString());
            return v;

    });

    QVectorIterator vit(vector);
    while(vit.hasNext()){
        const QMap<QString,QVariant> m = vit.next();
        int id = m.value("id").toInt();
        QString caption = m.value("caption").toString();
        QString iconName = m.value("icon_name").toString();
        bool isNode = m.value("is_node").toBool();

        if(!isNode){
            QTreeWidgetItem *twi = new QTreeWidgetItem(twp);
            twi->setText(0,caption);

            QIcon icon;
            icon.addPixmap(QPixmap(iconName));
            twi->setIcon(0,icon);

            QJsonObject jo;
            jo.insert("id",id);
            jo.insert("caption",caption);
            twi->setData(0,Qt::UserRole+1,QVariant::fromValue(jo));

            //如果当前节点是目录
            recursionChildren(twi,id);
        }
    }

}


///初始化工具条上的功能
void NodeTreeDialog::initToolBar(){

}

///初始化表格
void NodeTreeDialog::initNodeButtonTable(){
    ui->tw_nodeModels->setShowGrid(false);

}

///create class menu with json data
///从json文件创建分类信息菜单
//void NodeTreeDialog::makeModelMenuItem(AICCTreeWidget *atw){
//    Config config(QApplication::applicationDirPath() + "/conf/model_menu.json");
//    QJsonObject jo_root = config.getJsonRoot();
//    QList<QPair<QString,QJsonObject>> list_root = orderedQJsonObject(jo_root);

//    for(int i =0 ;i<list_root.size();i++){
//        QTreeWidgetItem *twi = new QTreeWidgetItem(atw);
//        //一级分类
//        twi->setText(0,list_root[i].first);
//        //一级以下分类递归处理
//        recursionQJsonObjectModuleBrowser(list_root[i].first,list_root[i].second,twi);
//    }
//}

///旧方法，只在root位置创建分类信息创建属性结构的根目录分类
//void NodeTreeDialog::makeRootGroupItem(AICCTreeWidget *atw,const QString name,const QString text)
//{
//    QTreeWidgetItem *rootGroupMathOperations = new QTreeWidgetItem(atw);
//    rootGroupMathOperations->setText(0,text);
//    rootGroupMathOperations->setData(0,Qt::UserRole+1,QVariant::fromValue(name));
//}


///获得DataModelRegistry对象
void NodeTreeDialog::setNodeMap(QMap<QString,QSet<QString>> pnm){
    //    //    qDebug() << "setNodeMap: " << pnm.size() << "  QSet size: " << pnm["MathOperations"].size();
    _nodeMap = pnm;
}

///根据名称创建button
AICCToolButton * NodeTreeDialog::createToolButton( QString name,QString caption){

    AICCToolButton *tb = new AICCToolButton();
    if(caption.lastIndexOf("::")!=-1)
        caption.insert(caption.lastIndexOf("::")+2,"\n");
    tb->setText("\n\n"+caption);
    tb->setNodeName(name);
    tb->setNodeCaption(caption);
    tb->setToolTip(name);
    //    tb->setNodeIcon("/res/nodeIcon/math_add.png");
    return tb;
}

///点击树形节点后右侧按钮区域变化
void NodeTreeDialog::treeWidgetItemClicked(QTreeWidgetItem *item, int column){
    //    this->setFixedWidth(1140);
    //    this->setFixedHeight(624);

    //    ///清空表格内容，删除已有的按钮
    //    ui->tw_nodeModels->setRowCount(0);
    //    ui->tw_nodeModels->clearContents();

    //    ///增加点击分类对应的按钮
    //    /// 最多6列，根据按钮的总量决定行数
    //    int columnCount = 6;
    //    QString itemData = item->data(0,Qt::UserRole+1).value<QString>();
    //    if(_nodeMap.contains(itemData)){
    //        AICCSqlite sqlite;
    //        QSet<QString> nodes = _nodeMap[itemData];

    //        int ncount = nodes.count();
    //        ui->tw_nodeModels->setRowCount(ncount/8+1);

    //        int i=0;
    //        foreach(QString name,nodes){
    //            QSqlQuery squery = sqlite.query(QString("select caption,node_icon from node where name = '%0'").arg(name));
    //            AICCToolButton *tb;
    //            if(squery.next()){
    //                tb = createToolButton(name,squery.value(0).toString());
    //                tb->setNodeIcon(squery.value(1).toString());
    //            }
    //            else{
    //                tb = createToolButton(name,name);
    //                tb->setNodeIcon("/res/nicon/node.png");
    //            }

    //            //点击按钮通知事件
    //            connect(tb,&QToolButton::clicked,this,[tb,this](bool checked=false){
    //                emit nodeDoubleClicked(tb->text());
    //            });
    //            ui->tw_nodeModels->setCellWidget(i/columnCount,i%columnCount,tb);
    //            i++;
    //        }
    //    }
}























