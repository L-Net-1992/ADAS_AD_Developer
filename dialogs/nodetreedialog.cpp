#include "nodetreedialog.h"
#include "ui_nodetreedialog.h"

NodeTreeDialog::NodeTreeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeTreeDialog)
{
    ui->setupUi(this);

    this->initTreeWidget();
    this->initToolBar();
    this->initNodeButtonTable();
}

NodeTreeDialog::~NodeTreeDialog()
{
    delete ui;
}
///初始化属性结构内容
void NodeTreeDialog::initTreeWidget()
{
    AICCTreeWidget *tw = ui->tw_nodeTree;
    tw->clear();

    //initToolBar();
    makeModelMenuItem(tw);

    //原一层分类目录
//    AICCSqlite sqlite;
//    QSqlQuery squery = sqlite.query("select class_name,class_desc from nodeClass order by sort");
//    while(squery.next()){
//        QString className = squery.value(0).toString();
//        QString classDesc = squery.value(1).toString();
//        makeRootGroupItem(tw,className,classDesc);
//    }

    connect(ui->tw_nodeTree,&AICCTreeWidget::itemClicked,this,&NodeTreeDialog::treeWidgetItemClicked);

    tw->expandAll();
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
void NodeTreeDialog::makeModelMenuItem(AICCTreeWidget *atw){
    Config config(QApplication::applicationDirPath() + "/conf/model_menu.json");
    QJsonObject jo_root = config.getJsonRoot();
    QList<QPair<QString,QJsonObject>> list_root = orderedQJsonObject(jo_root);

    for(int i =0 ;i<list_root.size();i++){
        QTreeWidgetItem *twi = new QTreeWidgetItem(atw);
        //一级分类
        twi->setText(0,list_root[i].first);
        //一级以下分类递归处理
        recursionQJsonObjectModuleBrowser(list_root[i].first,list_root[i].second,twi);
    }
}

///旧方法，只在root位置创建分类信息创建属性结构的根目录分类
void NodeTreeDialog::makeRootGroupItem(AICCTreeWidget *atw,const QString name,const QString text)
{
    QTreeWidgetItem *rootGroupMathOperations = new QTreeWidgetItem(atw);
    rootGroupMathOperations->setText(0,text);
    rootGroupMathOperations->setData(0,Qt::UserRole+1,QVariant::fromValue(name));
}


///获得DataModelRegistry对象
void NodeTreeDialog::setNodeMap(QMap<QString,QSet<QString>> pnm){
    qDebug() << "setNodeMap: " << pnm.size() << "  QSet size: " << pnm["MathOperations"].size();
    _nodeMap = pnm;
}

///根据名称创建button
AICCToolButton * createToolButton( QString name,QString caption){

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
    this->setFixedWidth(1140);
    this->setFixedHeight(624);

    ///清空表格内容，删除已有的按钮
    ui->tw_nodeModels->setRowCount(0);
    ui->tw_nodeModels->clearContents();

    ///增加点击分类对应的按钮
    /// 最多6列，根据按钮的总量决定行数
    int columnCount = 6;
    QString itemData = item->data(0,Qt::UserRole+1).value<QString>();
    if(_nodeMap.contains(itemData)){
        AICCSqlite sqlite;
//        void recursionQJsonObject(QString parentName,QJsonObject jo,QTreeWidgetItem *twi);
        QSet<QString> nodes = _nodeMap[itemData];

        int ncount = nodes.count();
        ui->tw_nodeModels->setRowCount(ncount/8+1);

        int i=0;
        foreach(QString name,nodes){
            QSqlQuery squery = sqlite.query(QString("select caption,node_icon from node where name = '%0'").arg(name));
            AICCToolButton *tb;
            if(squery.next()){
                tb = createToolButton(name,squery.value(0).toString());
                tb->setNodeIcon(squery.value(1).toString());
            }
            else{
                tb = createToolButton(name,name);
                tb->setNodeIcon("/res/nicon/node.png");
            }

            //点击按钮通知事件
            connect(tb,&QToolButton::clicked,this,[tb,this](bool checked=false){
               emit nodeDoubleClicked(tb->text());
            });
            ui->tw_nodeModels->setCellWidget(i/columnCount,i%columnCount,tb);
            i++;
        }
    }
}























