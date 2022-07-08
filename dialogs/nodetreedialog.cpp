#include "nodetreedialog.h"
#include "ui_nodetreedialog.h"
#include <QSqlRecord>
NodeTreeDialog::NodeTreeDialog(QSharedPointer<CategoryDataModel> cdm,QSharedPointer<ProjectDataModel> pdm,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeTreeDialog),
    _categoryDataModel(cdm),
    _projectDataModel(pdm)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_QuitOnClose, false);
    this->initToolBar();
    this->initTreeWidget();
    this->initNodeButtonTable();

}

NodeTreeDialog::~NodeTreeDialog()
{
    delete ui;
}

///初始化工具条上的功能
void NodeTreeDialog::initToolBar(){
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &NodeTreeDialog::modelSearchUpdate);
    connect(ui->tb_search_subsystem, &QToolButton::clicked, this, &NodeTreeDialog::modelSearchUpdate);
    connect(ui->tb_left_arrow, &QToolButton::clicked, this, [=](){
        int idx = ui->comboBox->currentIndex();
        int max = ui->comboBox->count();
        if(idx == 0) {
            ui->comboBox->setCurrentIndex(max-1);
        }
        else {
            ui->comboBox->setCurrentIndex(idx-1);
        }
    });
    connect(ui->tb_right_arrow, &QToolButton::clicked, this, [=](){
        int idx = ui->comboBox->currentIndex();
        int max = ui->comboBox->count();
        if(idx == max-1) {
            ui->comboBox->setCurrentIndex(0);
        }
        else {
            ui->comboBox->setCurrentIndex(idx+1);
        }
    });
}


/**
 * @brief NodeTreeDialog::initTreeWidget    初始化分类数据
 */
void NodeTreeDialog::initTreeWidget()
{
    ui->tw_nodeTree->setAcceptDrops(true);
    ui->tw_nodeTree->clear();

    //数据有改变时刷新左侧功能树
    connect(_categoryDataModel.get(),&CategoryDataModel::dataLoadCompleted,this,[&](const QJsonObject json){
            initNodeTree(ui->tw_nodeTree,json);
    });
    //项目加载完毕
    connect(_projectDataModel.get(),&ProjectDataModel::projectDataModelLoadCompleted,this,[&](const QString pname,const QString ppath){
        if(ui->tw_nodeTree->topLevelItem(0)!=Q_NULLPTR)
            ui->tw_nodeTree->topLevelItem(0)->setText(0,pname);
    });


    ui->tw_nodeTree->expandAll();
    //点击功能树展示分类下所有模块
    connect(ui->tw_nodeTree,&QTreeWidget::itemClicked,this,&NodeTreeDialog::itemClickedAction);

    //拖放后重新展示分类下模块数据
    connect(ui->tw_nodeTree,&AICCTreeWidget::dropCompleted,this,[&](QTreeWidgetItem *item){
        qDebug() << " drop completed";
        //        tw->clearFocus();
        //        item->setSelected(true);
        this->itemClickedAction(item,0);
    });
}

/**
 * @brief NodeTreeDialog::itemClickedAction 点击目录展示目录中所有模块
 * @param item
 * @param column
 */
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
        v.insert("parentid",q.value("parentid").toString());
        v.insert("class_name",q.value("class_name").toString());
        v.insert("caption",q.value("caption").toString());
        v.insert("icon_name",q.value("icon_name").toString());
        return v;
    };
    AICCSqlite sqlite;
    //将结果集转换
    QString sql = QString("select id,parentid,class_name,caption,icon_name from modelNode where parentid = %0 and is_node=1").arg(jo.value("id").toInt());
    QVector<QMap<QString,QVariant>> vector = sqlite.query1(sql,func_ptr);

    ui->tw_nodeModels->setRowCount(vector.size()/columnCount+1);

    QVectorIterator vit(vector);
    int i=0;
    while(vit.hasNext()){
        AICCToolButton *tb;

        const QMap<QString,QVariant> m = vit.next();
        QString id = m.value("id").toString();
        QString parentid = m.value("parentid").toString();
        QString className = m.value("class_name").toString();
        QString caption = m.value("caption").toString();
        QString iconName = m.value("icon_name").toString();

        if(_categoryDataModel->existNode(className.toStdString())){
            tb = createToolButton(id,parentid,className,caption,iconName);
            ui->tw_nodeModels->setCellWidget(i/columnCount,i%columnCount,tb);
            i++;
        }
    }
}


/**
 * @brief NodeTreeDialog::initNodeTree  左侧菜单内容初始化
 * @param tw_root
 * @param json
 */
void NodeTreeDialog::initNodeTree(AICCTreeWidget * tw_root,const QJsonObject json){
    if(tw_root==Q_NULLPTR)
        return;
    tw_root->clear();

    QTreeWidgetItem *twi_root = new QTreeWidgetItem(tw_root);
    //暂时隐藏
    twi_root->setText(0,"模块分类");
//    twi_root->setText(0,_projectDataModel->projectName());
//    QIcon icon;
//    icon.addPixmap(QPixmap(":/res/Open.png"));
//    twi_root->setIcon(0,icon);

    recursionChildren(json,twi_root,0);
    ui->tw_nodeTree->expandAll();
}

///递归所有子级节点
void NodeTreeDialog::recursionChildren(QJsonObject json,QTreeWidgetItem *twp,int pid){
    //1:获得当前级别的id排序数据
    QJsonArray ja = json.value("childSort").toArray();
    //2:根据排序的id处理每个节点的内容
    for(const QJsonValue jv:ja){
        //3:获得当前id的内容
        int kid = jv.toInt();

        QJsonObject jnode = json.value(QString::number(kid)).toObject();

        //4:处理数据库中获得的数据
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




/**
 * @brief NodeTreeDialog::initNodeButtonTable   初始化按钮表格
 */
void NodeTreeDialog::initNodeButtonTable(){

    //处理右键菜单
    QTableWidget *tw = ui->tw_nodeModels;

    tw->setShowGrid(false);
    tw->setContextMenuPolicy(Qt::CustomContextMenu);
    int selRow = -1;
    int selCol = -1;
    connect(tw,&QTableWidget::customContextMenuRequested,this,[&](QPoint p){
        QTableWidget *twc = ui->tw_nodeModels;
        QModelIndex mindex = twc->indexAt(p);
        QMenu *popMenu = new QMenu(twc);
        if(mindex.isValid()){
            QAction *action = new QAction();
            action->setText(QString(QStringLiteral("修改模块信息")));
            popMenu->addAction(action);

            connect(action,&QAction::triggered,this,[&](){
                AICCToolButton *button = (AICCToolButton*)twc->cellWidget(twc->currentRow(),twc->currentColumn());
                if(button == Q_NULLPTR) return;

                //获得包名第一个冒号之前为包名
                QRegExp reg_package("(::[a-zA-Z0-9]+)+");
                const QString package = button->nodeName().split(reg_package).at(0);
                //获得类名第一个冒号之后为名字空间+类名
                QRegExp reg("^([a-zA-Z0-9]+::)");
                const QString name = button->nodeName().split(reg).at(1);

                const QString bparentid = button->nodeParentId();
                const QString bcaption = button->nodeCaption();

                NewSubsystemDialog nsdialog(this);
                nsdialog.setCategoryComboBox(this->_categoryDataModel->currentUseCategoryFullPath());
                nsdialog.selectCategoryComboBox(bparentid.toInt());
                nsdialog.setPackageNameCaption(package,name,bcaption);

                NewSubsystemDialog::SubsystemDataModel subsystem_model;
                for(;;){
                    if(!nsdialog.exec()) return;
                    subsystem_model = nsdialog.getSubsystemDataModel();
                    //判断caption是否为空
                    if(subsystem_model.at("caption").empty()){
                        QMessageBox::critical(this,"错误","名称字段不能为空");
                        continue;
                    }
                    break;
                }

                QString id = button->nodeId();
                int parentid = QString::fromStdString(subsystem_model.at("parentid")).toInt();
                QString caption = QString::fromStdString(subsystem_model.at("caption"));

                AICCSqlite sqlite;
                QString sql = QString("update modelNode set caption = '%0',parentid = '%1' where id = %2").arg(caption).arg(parentid).arg(id);
                QSqlQuery query = sqlite.query(sql);

                AICCTreeWidget *tnt = ui->tw_nodeTree;
                //执行成功后刷新选择
                if(query.exec()){
                    if(tnt->selectedItems().size()==1){
                        QTreeWidgetItem *twi = tnt->selectedItems().at(0);
                        this->itemClickedAction(twi,0);
                    }
                }

            });

            popMenu->exec(QCursor::pos());
        }

        QList<QAction*> list = popMenu->actions();
        foreach(QAction *pAction,list) delete pAction;
        delete popMenu;
    });

    connect(tw,&QTableWidget::cellClicked,this,[&](int row,int col){
        selRow = row;
        selCol = col;
        qDebug() << row << col;
    });
}

/**
 * @brief NodeTreeDialog::createToolButton  创建表格中的模块按钮
 * @param id
 * @param parentid
 * @param name
 * @param caption
 * @param iconName
 * @return
 */
AICCToolButton * NodeTreeDialog::createToolButton(QString id,QString parentid, QString name,QString caption,QString iconName){
    AICCToolButton *tb = new AICCToolButton();
    tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    if(caption.lastIndexOf("::")!=-1)
        caption.insert(caption.lastIndexOf("::")+2,"\n");
    tb->setText(caption);
    tb->setToolTip(caption+"\n"+name);
    tb->setNodeIcon(iconName);
    tb->setNodeId(id);
    tb->setNodeParentId(parentid);
    tb->setNodeName(name);
    tb->setNodeCaption(caption);

    return tb;
}

// 模块浏览器搜索槽函数
void NodeTreeDialog::modelSearchUpdate()
{
    // 如果是空，退出
    if(ui->comboBox->currentText().isEmpty()) {
        return;
    }

    // 去除多余空格
    auto key = ui->comboBox->currentText().replace(QRegExp("[\\s]+"), " ").split(" ");
    key.removeAll("");
    if(key.isEmpty()) {
        return;
    }

    QString str;
    switch(key.size())
    {
    case 1:
        str = QString("SELECT * FROM modelNode WHERE caption like '%%1%'").arg(key.at(0));
        break;
    case 2:
        str = QString("SELECT * FROM modelNode WHERE caption like '%%1%' caption like '%%2%'").arg(key.at(0)).arg(key.at(1));
        break;
    default:
        str = QString("SELECT * FROM modelNode WHERE caption like '%%1%' caption like '%%2%'").arg(key.at(0)).arg(key.at(1));
        break;
    }

    //清空表格内容，删除已有的按钮
    ui->tw_nodeModels->setRowCount(0);
    ui->tw_nodeModels->clearContents();

    AICCSqlite sqlite;
    auto query = sqlite.query(str);
    int size = 0;
    while(query.next())
    {
        QSqlRecord rec = query.record();
        QString id = rec.value("id").toString();
        QString parentid = rec.value("parentid").toString();
        QString className = rec.value("class_name").toString();
        QString caption = rec.value("caption").toString();
        QString iconName = rec.value("icon_name").toString();

        if(size%6 == 0) {
            ui->tw_nodeModels->setRowCount(ui->tw_nodeModels->rowCount()+1);
        }
        AICCToolButton *tb;
        if(_categoryDataModel->existNode(className.toStdString())){
            tb = createToolButton(id,parentid,className,caption,iconName);
            ui->tw_nodeModels->setCellWidget(size/6,size%6,tb);
            size++;
        }
    }

    // 去除当前item高亮显示
    if(ui->tw_nodeTree->currentItem()) {
        ui->tw_nodeTree->currentItem()->setSelected(false);
    }
}

