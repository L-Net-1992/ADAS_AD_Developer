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
    tw->setAcceptDrops(true);
    tw->clear();

    initNodeTree(tw);
    tw->expandAll();
    connect(tw,&QTreeWidget::itemClicked,this,&NodeTreeDialog::itemClickedAction);
    connect(tw,&AICCTreeWidget::dropCompleted,this,[&](QTreeWidgetItem *item){
        qDebug() << " drop completed";
//        tw->clearFocus();
//        item->setSelected(true);
        this->itemClickedAction(item,0);
    });
}

///点击目录展示目录中所有模块
void NodeTreeDialog::itemClickedAction(QTreeWidgetItem *item,int column){
    qDebug() << "item click:" << item->text(0);
    this->setFixedWidth(1140);
    this->setFixedHeight(624);

    //清空表格内容，删除已有的按钮
    ui->tw_nodeModels->setRowCount(0);
    ui->tw_nodeModels->clearContents();

    //增加点击分类对应的按钮
    //最多6列，根据按钮的总量决定行数
    int columnCount = 6;
    //获得按钮相关数据
    QVariant v = item->data(0,Qt::UserRole+1);
    QJsonObject jo = v.toJsonObject();

    auto func_ptr = [](QSqlQuery q){
        QMap<QString,QVariant> v;
        v.insert("id",q.value("id").toString());
        v.insert("class_name",q.value("class_name").toString());
        v.insert("caption",q.value("caption").toString());
        v.insert("icon_name",q.value("icon_name").toString());
        return v;
    };
    AICCSqlite sl;
    //将结果集转换
    QString sql = QString("select id,class_name,caption,icon_name from modelNode where parentid = %0 and is_node=1").arg(jo.value("id").toInt());
    QVector<QMap<QString,QVariant>> vector =
            sl.query1(sql,func_ptr);


    ui->tw_nodeModels->setRowCount(vector.size()/columnCount+1);


    QVectorIterator vit(vector);
    int i=0;
    while(vit.hasNext()){
        AICCToolButton *tb;

        const QMap<QString,QVariant> m = vit.next();
        QString id = m.value("id").toString();
        QString className = m.value("class_name").toString();
        QString caption = m.value("caption").toString();
        QString iconName = m.value("icon_name").toString();
        tb = createToolButton(id,className,caption,iconName);
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

///获得DataModelRegistry对象
void NodeTreeDialog::setNodeMap(QMap<QString,QSet<QString>> pnm){
    //    //    qDebug() << "setNodeMap: " << pnm.size() << "  QSet size: " << pnm["MathOperations"].size();
    _nodeMap = pnm;
}

///根据名称创建button
AICCToolButton * NodeTreeDialog::createToolButton(QString id, QString name,QString caption,QString iconName){
    AICCToolButton *tb = new AICCToolButton();
    tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    if(caption.lastIndexOf("::")!=-1)
        caption.insert(caption.lastIndexOf("::")+2,"\n");
    tb->setText(caption);
    tb->setToolTip(caption+"\n"+name);
    tb->setNodeIcon(iconName);
    tb->setNodeId(id);
    tb->setNodeName(name);
    tb->setNodeCaption(caption);

    return tb;
}
























