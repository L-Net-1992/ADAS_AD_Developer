#include "nodetreedialog.h"
#include "ui_nodetreedialog.h"

NodeTreeDialog::NodeTreeDialog(QSharedPointer<CategoryDataModel> cdm,QSharedPointer<ProjectDataModel> pdm,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeTreeDialog),
    _categoryDataModel(cdm),
    _projectDataModel(pdm)
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
    ui->tw_nodeTree->setAcceptDrops(true);
    ui->tw_nodeTree->clear();

//    initNodeTree(ui->tw_nodeTree,_categoryDataModel->category());

    //数据有改变时刷新左侧功能树
    connect(_categoryDataModel.get(),&CategoryDataModel::dataLoadCompleted,this,[&](const QJsonObject json){
            initNodeTree(ui->tw_nodeTree,json);
    });

    connect(_projectDataModel.get(),&ProjectDataModel::projectDataModelLoadCompleted,this,[&](const QString pname,const QString ppath){
        if(ui->tw_nodeTree->topLevelItem(0)!=Q_NULLPTR)
            ui->tw_nodeTree->topLevelItem(0)->setText(0,pname);
    });


    ui->tw_nodeTree->expandAll();
    connect(ui->tw_nodeTree,&QTreeWidget::itemClicked,this,&NodeTreeDialog::itemClickedAction);
    connect(ui->tw_nodeTree,&AICCTreeWidget::dropCompleted,this,[&](QTreeWidgetItem *item){
        qDebug() << " drop completed";
        //        tw->clearFocus();
        //        item->setSelected(true);
        this->itemClickedAction(item,0);
    });
}

///点击目录展示目录中所有模块//    initNodeTree(tw);
void NodeTreeDialog::itemClickedAction(QTreeWidgetItem *item,int column){
//    qDebug() << "item click:" << item->text(0);
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
    AICCSqlite sqlite;
    //将结果集转换
    QString sql = QString("select id,class_name,caption,icon_name from modelNode where parentid = %0 and is_node=1").arg(jo.value("id").toInt());
    QVector<QMap<QString,QVariant>> vector = sqlite.query1(sql,func_ptr);


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

        if(_categoryDataModel->existNode(className.toStdString())){
            tb = createToolButton(id,className,caption,iconName);
            ui->tw_nodeModels->setCellWidget(i/columnCount,i%columnCount,tb);
        }

        i++;
    }
}

///左侧属性菜单部分_sqlite
void NodeTreeDialog::initNodeTree(AICCTreeWidget * tw_root,const QJsonObject json){
    if(tw_root==Q_NULLPTR)
        return;
    tw_root->clear();

    QTreeWidgetItem *twi_root = new QTreeWidgetItem(tw_root);
    //暂时隐藏
//    twi_root->setText(0,_projectDataModel->projectName());
//    QIcon icon;
//    icon.addPixmap(QPixmap(":/res/Open.png"));
//    twi_root->setIcon(0,icon);

    recursionChildren(json,twi_root,0);
    ui->tw_nodeTree->expandAll();
}

///递归所有子级节点
void NodeTreeDialog::recursionChildren(QJsonObject json,QTreeWidgetItem *twp,int pid){
    for(const QString &key:json.keys()){
        QJsonObject jnode = json.value(key).toObject();
        int id = jnode.value("id").toInt();
        QString caption = jnode.value("caption").toString();
        QString iconName = jnode.value("icon_name").toString();
        int isNode = jnode.value("is_node").toInt();

        if(isNode==0 && id !=0){
            QTreeWidgetItem *twi = new QTreeWidgetItem(twp);
            twi->setText(0,caption);

            QIcon icon;
            icon.addPixmap(QPixmap(iconName));
            twi->setIcon(0,icon);

            QJsonObject jo;
            jo.insert("id",id);
            jo.insert("caption",caption);
            twi->setData(0,Qt::UserRole+1,QVariant::fromValue(jo));

            recursionChildren(jnode,twi,id);
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
























