#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    projectDialog(new ProjectDialog(parent)),
    npDialog(new NodeParametersDialog(parent)),
    npmilDialog ( new NodeParametersMILDialog(parent)),
    nodeTreeDialog ( new NodeTreeDialog(parent)),
    isDialog (new ImportScriptDialog(parent)),
    diDialog ( new DataInspectorDialog(parent)),
    monitorDialog ( new MonitorDialog(parent)),
    eDialog ( new EditorWindow(parent)),
    cDialog ( new CalibrationDialog(parent)),
    process ( new QProcess(parent))

{
    qDebug() << "debug----------------";

    auto AA_modules_path = getenv("AA_PLATFORM_MODULES_PATH");
    if (nullptr != AA_modules_path) {
        std::cout << "AA_PLATFORM_MODULES_PATH = " << AA_modules_path << std::endl;
        modules_path_ = AA_modules_path;
    } else {
        std::cout << "AA_PLATFORM_MODULES_PATH is nullptr " << std::endl;
    }
    sqlite.initDatabaseConnection();
    //    _moduleLibrary = QSharedPointer<ModuleLibrary>(new ModuleLibrary());
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized);
    this->setAttribute(Qt::WA_QuitOnClose);
    pDataModel=new ProjectDataModel;
    rProjectDialog=new RecentProjectDialog(pDataModel,parent);


    MainWindow::pte_out = ui->pte_output;
    qInstallMessageHandler(logOutput);

    //获得Process的标准输出
    connect(process,&QProcess::readyRead,this,[&](){
        ui->pte_output->appendPlainText(process->readAll());
    });


    this->initMenu();
    this->initTreeView();
    this->setAcceptDrops(true);
    this->initSplitter();
    this->initTableWidget();
    this->initToolbar();
    this->initBreadcrumbNavigation();
    this->initStackedWidget();
    this->initNodeEditor();
    this->initImportScriptDialog();
    this->initProjectDialog();
    this->initRecentProjectDialog();

}

MainWindow::~MainWindow()
{
    sqlite.closeConnection();
    delete ui;
    delete pDataModel;
}

///初始化菜单
void MainWindow::initMenu()
{
    connect(ui->actionExit,&QAction::triggered,this,&QWidget::close);
    connect(ui->actionNodeWindow,&QAction::toggled,ui->dw_left,&QDockWidget::setVisible);
    connect(ui->actionPropertyWindow,&QAction::toggled,ui->dw_right,&QDockWidget::setVisible);
    connect(ui->actionAbout,&QAction::triggered,this,&QApplication::aboutQt);
    connect(ui->actionNewProject,&QAction::triggered,projectDialog,&ProjectDialog::show);
    connect(ui->actionOpen,&QAction::triggered,this,[&]{this->pbOpenAction();});
    //    connect(ui->actionCreateSubSystem,&QAction::triggered,this,[&]{this->createSubsysetmAction();});
}

void MainWindow::setTreeNode(QTreeWidget *tw,const char* ptext,const char* picon){
    QTreeWidgetItem *pItem = new QTreeWidgetItem();
    pItem->setText(0,ptext);
    QIcon icon;
    icon.addPixmap(QPixmap(picon));
    pItem->setIcon(0,icon);
    tw->addTopLevelItem(pItem);
}

void MainWindow::initTreeView()
{
    AICCTreeWidget *tw = ui->tw_node;
    tw->setDragDropMode(QAbstractItemView::DragOnly);
    tw->setDragEnabled(true);
    tw->clear();

    Config config(QApplication::applicationDirPath()+"/conf/model_menu.json");
    QJsonObject jo_root = config.getJsonRoot();
    QList<QPair<QString,QJsonObject>> list_root = orderedQJsonObject(jo_root);

    for(int i=0;i<list_root.size();i++){
        QTreeWidgetItem *twi = new QTreeWidgetItem(tw);
        twi->setText(0,list_root[i].first);
        //        twi->setBackground(0,QBrush(QColor("#FFFFFF")));
        recursionQJsonObjectLeft(list_root[i].first,list_root[i].second,twi);
    }

    ui->tw_node->expandAll();
}

///填充节点属性表格数据
void MainWindow::fillTableData(QTableWidget *tw,const NodeDataModel *ndm)
{
    tw->setRowCount(0);
    if(ndm==Q_NULLPTR) return;
    tw->setRowCount(tw->rowCount()+1);
    tw->setItem(0,0,new QTableWidgetItem("caption"));
    tw->setItem(0,1,new QTableWidgetItem(ndm->caption()));

    tw->setRowCount(tw->rowCount()+1);
    tw->setItem(1,0,new QTableWidgetItem("name"));
    tw->setItem(1,1,new QTableWidgetItem(ndm->name()));

    qDebug() << "emit getNodeDataModel";
}

void MainWindow::initSplitter()
{
    ui->dw_left->show();
    ui->dw_right->show();
}

///初始化右侧属性表格
void MainWindow::initTableWidget()
{
    ui->tw_nodeProperties->verticalHeader()->setHidden(true);
}

///初始化工具条
void MainWindow::initToolbar()
{
    ui->tb_merge_module->setVisible(false);

    //隐藏掉上方dock的标题栏
    QWidget *titleBarWidget = ui->dw_toolbar->titleBarWidget();
    QWidget *nullTitleBarWidget = new QWidget();
    ui->dw_toolbar->setTitleBarWidget(nullTitleBarWidget);
    delete titleBarWidget;

    ui->tw_toolbar->setCurrentIndex(0);

    //设置New按钮显示项目子窗口
    connect(ui->tb_new,&QToolButton::clicked,projectDialog,&ProjectDialog::show);

    //显示node模块的窗口
    connect(ui->pb_library_browser,&QPushButton::clicked,this,[&]{
        nodeTreeDialog->show();
    });

    //生成代码按钮
    connect(ui->pb_script_generator,&QPushButton::clicked,this,[&]{
        //generate cpp code
        AICCFlowView * fv = static_cast<AICCFlowView *>(ui->sw_flowscene->currentWidget());
        std::string generatePath = QApplication::applicationDirPath().append("/generate").toStdString();
        std::filesystem::path dir(generatePath);
        SourceGenerator::generateCMakeProject(dir,*(fv->scene()),_moduleLibrary->packageLibrary());

        //generate shell script
        SourceGenerator::generateScript(dir,(modules_path_+"/adas-target-jetson.json"),"jetson",*(fv->scene()),_moduleLibrary->packageLibrary());
        SourceGenerator::generateScript(dir,(modules_path_+"/adas-target-bst.json"),"bst",*(fv->scene()),_moduleLibrary->packageLibrary());
        SourceGenerator::generateScript(dir,(modules_path_+"/adas-target-mdc.json"),"mdc",*(fv->scene()),_moduleLibrary->packageLibrary());

        generatePath.append("/generate.cpp");
        eDialog->openTextFile(QString::fromStdString(generatePath));
        eDialog->show();

    },Qt::UniqueConnection);

    //导入脚本按钮
    connect(ui->tb_import,&QPushButton::clicked,this,[&]{
        isDialog->show();
    });

    //打开按钮响应动作
    connect(ui->pb_open,&QPushButton::clicked,this,[&]{pbOpenAction();});

    //保存按钮响应动作，当前只保存一个NodeEditor的内容，子系统实现后需要保存多个NodeEditor内容
    connect(ui->pb_save,&QPushButton::clicked,this,[&]{
        //0：判断当前是否为未关联项目，如未关联项目要求用户先创建项目
        if(this->pDataModel->currentProjectName()==""){
            int result  = projectDialog->exec();
            if(result==QDialog::Rejected) return;
        }

        //1：加载项目文件，初始化所有项目数据
        //        QString fileName = QFileDialog::getOpenFileName(this,tr("Open Project"),QDir::homePath(),tr("Project (*.xml)"));
        QString fileName = pDataModel->currentProjectPath()+"/"+pDataModel->currentProjectName()+"/.ap/project.xml";
        if(!QFileInfo::exists(fileName)) return;
        QFile file(fileName);
        pDataModel->readProjectXml(file);

        //2：保存当前内容到flow文件中
        AICCFlowView *fv = static_cast<AICCFlowView *>(ui->sw_flowscene->currentWidget());
        qDebug() << pDataModel->currentProjectPath()<< "   "<< pDataModel->currentProjectName();
        if(pDataModel->currentProjectPath()=="" || pDataModel->currentProjectName()==""){
            QMessageBox::critical(Q_NULLPTR,"critical","请先选择项目再进行保存",QMessageBox::Ok,QMessageBox::Ok);
            return;
        }

        for(const QString &ssf :pDataModel->currentFlowSceneSaveFiles()){
            QString saveFileName = pDataModel->currentProjectPath()+"/"+pDataModel->currentProjectName()+"/"+ssf;
            qDebug() << "save file name:" << saveFileName;
            QFile file(saveFileName);
            if(file.open(QIODevice::WriteOnly)){
                file.write(fv->scene()->saveToMemory());
            }
        }
    });

    ///点击显示数据检查器窗口
    connect(ui->pb_dataInspector,&QPushButton::clicked,this,[&](){
        //        diDialog->show();
        monitorDialog->show();
    });

    ///平台选择下拉框
    connect(ui->cb_select_platform,&QComboBox::currentTextChanged,this,[&](const QString &text){
        if(text==QString::fromLocal8Bit("SelectPlatform"))
            ui->tb_code_compiler->setEnabled(false);
        else
            ui->tb_code_compiler->setEnabled(true);
    });

    ///显示脚本编辑器
    connect(ui->tb_edit_script,&QToolButton::clicked,this,[&]{
        eDialog->show();
    });


    ///代码编译按钮code compiler
    connect(ui->tb_code_compiler,&QToolButton::clicked, this,[&](){
        QVector<QString> v;
        v << "build_bst.sh" << "build_jetson.sh" << "build_mdc.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });


    ///deploy
    connect(ui->tb_script_deploy,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "deploy_bst.sh" << "deploy_jetson.sh" << "deploy_mdc.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///run
    connect(ui->tb_run,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "run_bst.sh" << "run_jetson.sh" << "run_mdc.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///stop
    connect(ui->tb_stop,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "stop_bst.sh" << "stop_jetson.sh" << "stop_mdc.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///在线标定按钮OnlineCalibration->Calibration
    connect(ui->tb_calibration,&QToolButton::clicked,this,[&](){
        cDialog->show();
    });

    ///注入按钮 仿真->注入
    connect(ui->tb_inject,&QToolButton::clicked,this,[&]{
        //生成node节点
        FlowView *view = static_cast<AICCFlowView *>(ui->sw_flowscene->currentWidget());
        FlowScene *scene = static_cast<AICCFlowView *>(ui->sw_flowscene->currentWidget())->scene();
        auto type = scene->registry().create("business_package::mil_import");

        //定位生成位置
        QRectF rect = view->sceneRect();
        float x = rect.x()+rect.width()/2;
        float y = rect.y()+rect.height()/5;

        if(type){
            auto& node = scene->createNode(std::move(type));
            node.nodeGraphicsObject().setPos(QPointF(x,y));
            scene->nodePlaced(node);
            this->npmilDialog->show();
        }
    });

    ///创建子系统按钮
    connect(ui->tb_create_subsystem,&QToolButton::clicked,this,[&]{
        _moduleLibrary->newSubsystem(this);
    });
    ///测试dialog显示
    //    connect(ui->pb_modelSettings,&QPushButton::clicked,this,[&](){
    //        TestDialog *tdialog = new TestDialog(this);
    //        tdialog->show();
    //    });
}

void MainWindow::processStart(const QVector<QString> scriptNames,const int platformIndex){
    QString bash="bash ";
    bash.append(QApplication::applicationDirPath()).append("/generate/").append(scriptNames[platformIndex-1]);
    QString killprocess = "kill -9 $(ps -ef|grep adas_generate|grep -v grep|awk '{print $2}')";
    process->terminate();
    if(process->waitForFinished())
        process->start(bash);
    else
        process->start(killprocess);
}

///初始化面包屑导航
void MainWindow::initBreadcrumbNavigation(){
    QStringList lpath;
    lpath << "根目录";
    ui->l_breadcrumb_navigation->makeNavigationData(lpath);
    ui->l_breadcrumb_navigation->refreshNavigationView();

    ///相应点击链接的操作
    connect(ui->l_breadcrumb_navigation,&AICCBreadcrumbNavigation::linkActivated,this,[&](const QString &link){
        ui->sw_flowscene->setCurrPagePathName(link);
        qDebug() << "clicked link:" << link ;
    });
}

///初始化StackedWidget
void MainWindow::initStackedWidget(){
    ///nodeeditor数据注册完后，要将数据传递给NodeTreeDialog窗口使用
    connect(ui->sw_flowscene,&AICCStackedWidget::registerDataModelsCompleted,this,[&](const QMap<QString,QSet<QString>> nodeMap){
        nodeTreeDialog->setNodeMap(nodeMap);
    });

    ///获得数据后填充右侧属性窗口
    connect(ui->sw_flowscene,&AICCStackedWidget::getNodeDataModel,this,[&](NodeDataModel *ndm){
        QTableWidget *tw = ui->tw_nodeProperties;
        tw->setRowCount(0);
        if(ndm==Q_NULLPTR) return;
        tw->setRowCount(tw->rowCount()+1);
        tw->setItem(0,0,new QTableWidgetItem("caption"));
        tw->setItem(0,1,new QTableWidgetItem(ndm->caption()));

        tw->setRowCount(tw->rowCount()+1);
        tw->setItem(1,0,new QTableWidgetItem("name"));
        tw->setItem(1,1,new QTableWidgetItem(ndm->name()));
    });


    ///双击node节点模块后弹出显示属性的窗口
    connect(ui->sw_flowscene,&AICCStackedWidget::nodeDoubleClicked,this,[&](NodeDataModel *nodeDataModel,const QString &pagePathName){
        //如果点击的是子窗口模块不进行处理
        auto * invocableDataModel = static_cast<InvocableDataModel*>(nodeDataModel);
        const auto & invocable = invocableDataModel->invocable();
        if(invocable.getType() == Invocable::Subsystem){
            _moduleLibrary->openSubsystem(this, invocable.getPackage(), invocable.getSubsystemName());

        }
        //如果点击的是MIL模块，显示特殊的文件加载子窗口
        else if(nodeDataModel->name()=="business_package::mil_import"){
            npmilDialog->show();
        }
        else {
            npDialog->show();
            QTableWidget *nptw =  npDialog->getTableNodeParameters();
            fillTableData(nptw,nodeDataModel);
        }
    });


    ///通知面包屑导航改变
    connect(ui->sw_flowscene,&AICCStackedWidget::notifyCurrentPagePathNameChanged,ui->l_breadcrumb_navigation,[&](const QString &url){
        ui->l_breadcrumb_navigation->makeNavigationData(url);
        ui->l_breadcrumb_navigation->refreshNavigationView();
    });

    ///向StackedWidget控件中增加第一个页面，并增加第一个FlowScene
    ui->sw_flowscene->addNewPageFlowScene("");

//    qDebug() << "stackwidget count:" << ui->sw_flowscene;

}


///打开项目动作函数部分
void MainWindow::pbOpenAction(QString projectPath){
    //0:数据准备
    FlowScene *scene =  static_cast<AICCFlowView *>(ui->sw_flowscene->currentWidget())->scene();
    //1：加载配置文件初始化各项数据
    QString fileName;
    if(projectPath==Q_NULLPTR){
        projectPath = QDir::homePath();
        fileName = QFileDialog::getOpenFileName(this,tr("打开项目"),QDir::homePath(),tr("项目 (*.xml)"));
    } else
        fileName = projectPath.append("/.ap").append("/project.xml");
    if(!QFileInfo::exists(fileName)) return;
    QFile file(fileName);
    pDataModel->readProjectXml(file);

    //2:判断是否有subsystem文件夹如果有则直接设置目录，否则创新的目录再进行设置
    //    QString psubsystem = pDataModel->currentProjectPath()+"/subsystem";
    QString psubsystem = pDataModel->currentProjectSubSystemPath();
    QDir dirSubsystem;
    if(!dirSubsystem.exists(psubsystem)){
        dirSubsystem.mkpath(psubsystem);
    }
    std::string sp = pDataModel->currentProjectSubSystemPath().toStdString();
    _moduleLibrary->setSubsystemPath(sp);
    _subsystemLibrary->setPath(sp);

    //3:加载当前项目的子系统模块
    for (const auto &inv: _subsystemLibrary->getInvocableList()) {
        auto f = [inv]() {
            std::unique_ptr<InvocableDataModel> p = std::make_unique<InvocableDataModel>(inv);
            p->setCaption(p->name());
            return p;
        };
        scene->registry().registerModel<InvocableDataModel>(f, "自定义模块");
    }

    //4：将名称为mainFlowScene的文件内容加载到主FlowScene上
    QString loadFileName = pDataModel->currentProjectPath()+"/"+pDataModel->currentProjectName()+"/"+ pDataModel->currentFlowSceneSaveFiles()[0];
    if (!QFileInfo::exists(loadFileName)) return;

    QFile loadFile(loadFileName);
    if(!loadFile.open(QIODevice::ReadOnly)) return;

    scene->clearScene();
    QByteArray wholeFile = loadFile.readAll();
    scene->loadFromMemory(wholeFile);
    this->setWindowTitle(pDataModel->currentProjectName()+" ("+pDataModel->currentProjectPath()+") "+" - 图形化ADAS/AD应用开发系");

//    qDebug() << "stackwidget count:" << ui->sw_flowscene;
}

///创建子系统
void MainWindow::createSubsysetmAction(){
    _moduleLibrary->newSubsystem(this);
}

///初始化最近打开项目窗口
void MainWindow::initRecentProjectDialog(){
    connect(rProjectDialog,&RecentProjectDialog::setCurrentProjectDataModelCompleted,this,[&](ProjectDataModel *pdm){
        this->pbOpenAction(pdm->currentProjectPath());
    });
    connect(rProjectDialog,&RecentProjectDialog::recentProjectDialogClosed,this,[&]{
        qDebug() <<" close ";
        this->close();

    });
    connect(rProjectDialog,&RecentProjectDialog::newProjectTriggered,this,[&]{
        projectDialog->show();
        rProjectDialog->hide();
    });
}

///初始化与项目创建窗口相关的内容
void MainWindow::initProjectDialog(){
    connect(projectDialog,&ProjectDialog::projectCreateCompleted,this,[&](bool success){
        //1:处理面包屑导航
        QStringList lpath;
        lpath << "根目录";
        ui->l_breadcrumb_navigation->makeNavigationData(lpath);
        ui->l_breadcrumb_navigation->refreshNavigationView();

        //2:初始化FlowScene
        ui->sw_flowscene->initDefaultScenes();

    });

    ///项目窗口关闭暂时不关闭主窗口
    //    connect(projectDialog,&ProjectDialog::projectDialogCanceled,this,[&]{
    //        this->close();
    //    });
}

///NodeEditor数据处理部分
//初始化时初始化主Scene的右键菜单，和NodeTreeDialog的node分类数据
//TODO:
void MainWindow::initNodeEditor(){
    _moduleLibrary = QSharedPointer<ModuleLibrary>(new ModuleLibrary);
    _subsystemLibrary = QSharedPointer<SubsystemLibrary>(new SubsystemLibrary);
    //1:解析pakage文件
    const QString path = QString::fromStdString(modules_path_); //QApplication::applicationDirPath()+"/install/";

    QStringList files = getADASPackagesFileList(path);
    //2:执行加载前的准备动作
    ui->statusbar->showMessage("正在加载节点模块数据");
    ui->tw_toolbar->setEnabled(false);
    ui->tw_node->setEnabled(false);

    //3:创建单独线程，耗时操作放到其中，防止界面卡死
    QtConcurrent::run([&,files](){
        try{
            _moduleLibrary->importFiles(files);
            std::list<Invocable> parserResult = _moduleLibrary->getParseResult();
            emit scriptParserCompleted(parserResult);
        }catch(const std::exception &e){
            qDebug() << "exception:" << e.what();
        }

    });
    qRegisterMetaType<std::list<Invocable>>("std::list<Invocable>");
    connect(this,&MainWindow::scriptParserCompleted,this,&MainWindow::registrySceneGenerateNodeMenu);


    //    connect(_moduleLibrary.get(),&ModuleLibrary::fileParserCompleted,this,[&](const int count ,const int index,const QString filename){
    //加载过程中显示当前进度
    //        ui->statusbar->showMessage("Loaded node modules:"+filename+"("+QString::number(index+1)+"/"+QString::number(count)+")",(index+1)>=count ? 3000 : 0);
    //        ui->statusbar->showMessage("")
    //    });


    /*old code

    //1:先解析package文件，准备好解析文件中的node数据
    const QString path = QApplication::applicationDirPath()+"/nodeconfig/";
    QStringList files = getFileList(path);

    //0:执行加载前准备动作
    ui->statusbar->showMessage("Start load node moduls data...");
    ui->tw_toolbar->setEnabled(false);

    //耗时操作放到单独线程中操作，操作完毕后通知外部继续执行
    QtConcurrent::run([&,files](){
        _moduleLibrary->importFiles(files);
        std::list<Invocable> parserResult = _moduleLibrary->getParseResult();
        emit scriptParserCompleted(parserResult);
    });

    //接收scriptParserCompleted信号，执行后续操作
    //为connect注册std::list<Invocable>类型，否则connect在SLOT中会不识别该类型
    qRegisterMetaType<std::list<Invocable>>("std::list<Invocable>");
    connect(this,&MainWindow::scriptParserCompleted,this,&MainWindow::registrySceneGenerateNodeMenu);
    //显示状态栏进度数据
    connect(_moduleLibrary.get(),&ModuleLibrary::fileParserCompleted,this,[&](const int count ,const int index,const QString filename){
        //加载过程中显示当前进度
        ui->statusbar->showMessage("Loaded node modules:"+filename+"("+QString::number(index+1)+"/"+QString::number(count)+")",(index+1)>=count ? 3000 : 0);
    });

    */
}

///生成右键菜单
void MainWindow::registrySceneGenerateNodeMenu(std::list<Invocable> parserResult){
    //1:生成scene的右键node数据,并注册到所有scene中
    std::shared_ptr<DataModelRegistry> registerDataModels = this->registerDataModels(parserResult);
    QList<AICCFlowView *> views =  ui->sw_flowscene->allViews();
    for(AICCFlowView *view:views){
        view->scene()->setRegistry(registerDataModels);
    }


    //2:生成NodeTreeDialog的node菜单结构
    QMap<QString,QSet<QString>> nodeCategoryDataModels = this->newNodeCategoryDataModels(parserResult);
    nodeTreeDialog->setNodeMap(nodeCategoryDataModels);

    //3:启用工具栏、展示选择项目窗口
    ui->statusbar->showMessage("节点模型数据加载已完成");
    ui->tw_toolbar->setEnabled(true);
    ui->tw_node->setEnabled(true);
    ui->menubar->setEnabled(true);
    rProjectDialog->show();
}

///初始化导入脚本对话框的内容
void MainWindow::initImportScriptDialog(){
    //选择文本后响应函数
    connect(isDialog,&ImportScriptDialog::filesSelected,this,[&](const QStringList files){
        QtConcurrent::run([&,files](){
            //1:解析选择文件中的node
            _moduleLibrary->importFiles(files);
            std::list<Invocable> parserResult = _moduleLibrary->getParseResult();
            //此处只通知initNodeEditor函数中链接的registrySceneGenerateNodeMenu函数执行后续操作即可
            emit scriptParserCompleted(parserResult);
        });
    });

    //文件解析百分比
    //TODO:此处可能要处理
    //    connect(_moduleLibrary.get(),&ModuleLibrary::fileParserCompleted,this,[&](const int count,const int index,const QString filename){
    //        isDialog->setImportProcess(index,count);
    //        isDialog->setListModels(_moduleLibrary.get());
    //    });
}

///初始化数据检查窗口
void MainWindow::initDataInspectorDialog(){

}

//只负责注册右键菜单，并返回右键菜单的数据模型
std::shared_ptr<DataModelRegistry> MainWindow::registerDataModels(const std::list<Invocable> parserResult){
    auto ret = std::make_shared<DataModelRegistry>();
    AICCSqlite sqlite;
    //注册所有已有的节点
    for(auto it = parserResult.begin();it!=parserResult.end();++it){
        const auto &inv = *it;
        QString sql = QString("select n.name,n.caption,nc.class_name from node n inner join nodeClass nc on n.class_id = nc.id where n.name = '%0'").arg(QString::fromStdString(inv.getName()));
        QSqlQuery squery = sqlite.query(sql);
        if(squery.next()){
            QString caption = squery.value(1).toString();
            QString className = squery.value(2).toString();
            auto f = [inv,caption](){
                std::unique_ptr<InvocableDataModel> p = std::make_unique<InvocableDataModel>(inv);

                p->setCaption(caption);
                return p;
            };

            ret->registerModel<InvocableDataModel>(f,className);
        }else{
            auto f = [inv](){
                std::unique_ptr<InvocableDataModel> p = std::make_unique<InvocableDataModel>(inv);
                p->setCaption(p->name());
                return p;
            };
            ret->registerModel<InvocableDataModel>(f,"Other");
        }

    }

    return ret;
}

///旧版只有一级分类信息，只负责NodeTreeDialog的node模块分类
//QMap<QString,QSet<QString>> MainWindow::nodeCategoryDataModels(const std::list<Invocable> parserResult){
//    QMap<QString,QSet<QString>> ret;
//    //定义写入分类数据函数
//    auto f_insertNodeCategoryMap = [&ret](const QString className,const QString nodeName){
//        QSet<QString> category;
//        category = ret.value(className);
//        category.insert(nodeName);
//        ret.insert(className,category);
//    };
//    AICCSqlite sqlite;
//    for(auto it = parserResult.begin();it!=parserResult.end();++it){
//        const auto &inv = *it;
//        QString sql = QString("select n.name,n.caption,nc.class_name from node n inner join nodeClass nc on n.class_id = nc.id where n.name = '%0'").arg(QString::fromStdString(inv.getName()));
//        QSqlQuery squery = sqlite.query(sql);
//        if(squery.next()){
//            QString className = squery.value(2).toString();
//            f_insertNodeCategoryMap(className,QString::fromStdString(inv.getName()));
//        }else{
//            f_insertNodeCategoryMap("Other",QString::fromStdString(inv.getName()));
//        }
//    }
//    return ret;
//}

///新版负责NodeTreeDialog的node模块分类信息
QMap<QString,QSet<QString>> MainWindow::newNodeCategoryDataModels(const std::list<Invocable> parserResult){
    QMap<QString,QSet<QString>> ret;
    //写入分类数据
    auto f_insertNodeClassMap = [&ret](const QString className,const QString nodeName){
        QSet<QString> cn;
        cn = ret.value(className);
        cn.insert(nodeName);
        ret.insert(className,cn);
    };
    AICCSqlite sqlite;
    for(auto it = parserResult.begin();it!=parserResult.end();++it){
        const auto &inv = *it;
        QString sql = QString("select n.name,n.caption,nc.class_name from node n inner join nodeClass nc on n.class_id = nc.id where n.name = '%0'").arg(QString::fromStdString(inv.getName()));
        QSqlQuery squery = sqlite.query(sql);

        if(squery.next()){
            QString className = squery.value(2).toString();
            QString nodeName = squery.value(0).toString();
            f_insertNodeClassMap(className,nodeName);
        }else{
            f_insertNodeClassMap("Other",QString::fromStdString(inv.getName()));
        }
    }
    return ret;
}

void MainWindow::logOutput(QtMsgType type,const QMessageLogContext &context,const QString &msg){
    QString omsg;
    omsg.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" ");
    switch(type){
    case QtInfoMsg:
    case QtDebugMsg:
        omsg.append("Debug:");
        break;
    case QtWarningMsg:
        omsg.append("Warning:");
        break;
    case QtCriticalMsg:
        omsg.append("Critical:");
        break;
    case QtFatalMsg:
        omsg.append("Fatal:");
    }
    omsg.append(msg);
    pte_out->appendPlainText(omsg);
}

void MainWindow::write(QString str){
    //    text = str;
}


