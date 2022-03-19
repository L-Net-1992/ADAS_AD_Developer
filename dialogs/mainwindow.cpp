#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    projectDialog(new ProjectDialog(parent)),
    npDialog(new NodeParametersDialog(parent)),
    npmilDialog ( new NodeParametersMILDialog(parent)),
    nodeTreeDialog ( new NodeTreeDialog(parent)),
    diDialog ( new DataInspectorDialog(parent)),
    eDialog ( new EditorWindow(parent)),
    process ( new QProcess(parent))

{
    sqlite.initDatabaseConnection();
    //    _moduleLibrary = QSharedPointer<ModuleLibrary>(new ModuleLibrary());
    ui->setupUi(this);

//    qInstallMessageHandler(logOutput);

    this->setWindowState(Qt::WindowMaximized);
    this->setAttribute(Qt::WA_QuitOnClose);
    pDataModel=new ProjectDataModel;
    rProjectDialog=new RecentProjectDialog(pDataModel,parent);
    isDialog =new ImportScriptDialog(pDataModel,parent);
    emDialog = new ExportModuleDialog(pDataModel,parent);



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

    //    qDebug() << "11111";
    //    qInfo() << "22222";

//    ui->pte_output->appendPlainText("cccc");
}

MainWindow::~MainWindow()
{
    sqlite.closeConnection();
    delete ui;
    delete pDataModel;
}

QString text;

void MainWindow::on_pte_output_textChanged(){
    //void MainWindow::on_pte_output_textChanged(){
//    qDebug() << "arg1-----" ;
//    ui->pte_output->appendPlainText(text);
}

void MainWindow::write(QString str){
    text = str;
}

void MainWindow::logOutput(QtMsgType type,const QMessageLogContext &context,const QString &msg){
    QString text;
    text.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" ");
    switch(type){
    case QtDebugMsg:
        text.append("Debug:");
        break;
    case QtWarningMsg:
        text.append("Warning:");
        break;
    case QtCriticalMsg:
        text.append("Critical:");
        break;
    case QtFatalMsg:
        text.append("Fatal:");
    }
    text.append(msg);
//    write(text);
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


///初始化左侧功能树
void MainWindow::initTreeView()
{
    ui->tw_node->fillInitLeftTreeData();
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
    ui->tb_import->setVisible(false);

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
    connect(ui->pb_script_generator,&QPushButton::clicked,this,[&](){
        //generate cpp code
        AICCFlowScene *scene = ui->sw_flowscene->getCurrentView()->scene();

        //不放到项目路径中，放到平台执行目录中，否则编译时找不到
        //        std::string generatePath = (pDataModel->currentProjectPath()+"/"+pDataModel->currentProjectName()+"/generate").toStdString();
        std::string generatePath = (QApplication::applicationDirPath()+"/generate").toStdString();
        std::filesystem::path dir(generatePath);
        SourceGenerator::generateCMakeProject(dir,*scene,*_moduleLibrary);

        //generate shell script
        SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/install/adas-target-jetson.json").toStdString(),"jetson",*scene,_moduleLibrary->packageLibrary());
        SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/install/adas-target-bst.json").toStdString(),"bst",*scene,_moduleLibrary->packageLibrary());
        SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/install/adas-target-mdc.json").toStdString(),"mdc",*scene,_moduleLibrary->packageLibrary());
        SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/install/adas-target-x86_64.json").toStdString(),"x86_64",*scene,_moduleLibrary->packageLibrary());

        generatePath.append("/generate.cpp");
        eDialog->openTextFile(QString::fromStdString(generatePath));
        eDialog->show();

    },Qt::UniqueConnection);

    //导入脚本按钮
    connect(ui->tb_import,&QPushButton::clicked,this,[&]{
        isDialog->show();
    });

    //导出子系统模块按钮
    connect(ui->tb_export_module,&QToolButton::clicked,this,[&]{
        emDialog->show();
    });
    //导入子系统模块按钮
    //导入一个flow文件成为自定义模块
    connect(ui->tb_import_module,&QToolButton::clicked,this,[&]{
        QString importModuleName = QFileDialog::getOpenFileName(this,tr("请选择要导入的自定义模块文件"),QApplication::applicationDirPath(),tr("自定义模块flow文件 (*.flow)"),Q_NULLPTR,QFileDialog::ReadOnly);
        QFileInfo imFileInfo(importModuleName);
        QString imPath =  imFileInfo.absolutePath();
        imPath.replace("\\", "/");
        qDebug() << "imPath: " << imPath.split("/").at(imPath.split("/").count()-1);
        QString imPackage = imPath.split("/").at(imPath.split("/").count()-1);

        //在项目的subsystem path目录下创建模块包
        QDir packageDir(pDataModel->currentProjectSubSystemPath()+"/"+imPackage);
        if (!packageDir.exists()) {
            if (!packageDir.mkdir(packageDir.absolutePath()))
                return false;
        }
        copyFile(importModuleName,packageDir.path()+"/"+imFileInfo.fileName(),true);
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
            qInfo() << "save file name:" << saveFileName;
            QFile file(saveFileName);
            if(file.open(QIODevice::WriteOnly)){
                file.write(fv->scene()->saveToMemory());
            }
        }
    });

    ///点击显示数据检查器窗口
    connect(ui->pb_dataInspector,&QPushButton::clicked,this,[&](){
        QJsonObject jo = getConfig();
        monitorDialog = new MonitorDialog(jo.value("deviceIP").toString(),this);
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
        v << "build_bst.sh" << "build_jetson.sh" << "build_mdc.sh" << "build_x86_64.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });


    ///deploy
    connect(ui->tb_script_deploy,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "deploy_bst.sh" << "deploy_jetson.sh" << "deploy_mdc.sh" << "deploy_x86_64.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///run
    connect(ui->tb_run,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "run_bst.sh" << "run_jetson.sh" << "run_mdc.sh" << "run_x86_64.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///stop
    connect(ui->tb_stop,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "stop_bst.sh" << "stop_jetson.sh" << "stop_mdc.sh" << "stop_x86_64.sh";
        processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///在线标定按钮OnlineCalibration->Calibration
    connect(ui->tb_calibration,&QToolButton::clicked,this,[&](){
        QJsonObject jo = getConfig();
        cDialog = new CalibrationDialog(jo.value("deviceIP").toString(),pDataModel,this);

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
    process->start(bash);

    //    bool ret = process->waitForFinished();
    //    if(ret){
    //        qDebug() << "process close";
    //        process->close();
    //        delete process;
    //    }
    //    if(process->waitForFinished())
    //        process->start(bash);
    //    else
    //        process->start(killprocess);
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

    //3:注册所有的功能模块到右键上
    std::shared_ptr<DataModelRegistry> dmr = registerDataModels();
    ui->sw_flowscene->getCurrentView()->scene()->setRegistry(dmr);

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

//void MainWindow::invocableParserAction(const std::string msg){
//    qDebug() << "=============================";
//    ui->pte_output->appendPlainText(QString::fromStdString(msg));

//}

///NodeEditor数据处理部分
//初始化时初始化主Scene的右键菜单，和NodeTreeDialog的node分类数据
//TODO:
void MainWindow::initNodeEditor(){
    //    _moduleLibrary = QSharedPointer<ModuleLibrary>(new ModuleLibrary);
    //    _subsystemLibrary = QSharedPointer<SubsystemLibrary>(new SubsystemLibrary(_moduleLibrary.get()));
    _moduleLibrary = new ModuleLibrary;
    //获得解析程序输出
    //    connect(_moduleLibrary,&ModuleLibrary::invocableParserCout,this,&MainWindow::invocableParserAction);

    _subsystemLibrary = new SubsystemLibrary;

    //1:解析pakage文件
    const QString path = QApplication::applicationDirPath()+"/install/";
    QStringList files = getADASPackagesFileList(path);

    //2:执行加载前的准备动作
    ui->statusbar->showMessage("正在加载节点模块数据");
    ui->tw_toolbar->setEnabled(false);
    ui->tw_node->setEnabled(false);

//    ui->pte_output->appendPlainText("--------------------");
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

    //4:包数据解析完成后续工作
    connect(this,&MainWindow::scriptParserCompleted,this,&MainWindow::scriptParserCompletedAction);

    //5:响应importCompleted
    //    connect(_moduleLibrary,&ModuleLibrary::importCompleted,this,&MainWindow::importCompletedAction);
    connect(_moduleLibrary,&ModuleLibrary::importCompleted,this,[&](){
        //        ui->pte_output->appendPlainText("模型数据加载完毕");
        ui->sw_flowscene->getCurrentView()->scene()->setRegistry(this->registerDataModels());
    });

    //6:响应文件解析进度
    connect(_moduleLibrary,&ModuleLibrary::fileParserCompleted,this,[&](int count, int index){
        //        qDebug() << " ==================================================processing:" << index << "/" << count;
    });

}

///导入完成响应动作，此处不要用lambda表达式，会导致跨线程调用问题
void MainWindow::importCompletedAction(){
    ui->sw_flowscene->getCurrentView()->scene()->setRegistry(this->registerDataModels());
}

///包数据解析完毕工作
void MainWindow::scriptParserCompletedAction(std::list<Invocable> parserResult){

    //1:生成NodeTreeDialog的菜单结构
    QMap<QString,QSet<QString>> nodeCategoryDataModels = this->newNodeCategoryDataModels(parserResult);
    nodeTreeDialog->setNodeMap(nodeCategoryDataModels);
    //2:初始化模块变量相关操作
    FlowScene *scene = ui->sw_flowscene->getCurrentView()->scene();
    connect(scene, &FlowScene::nodeCreated, [scene](QtNodes::Node & node){
        ModuleLibrary::generateVarNameIfEmpty(*scene, node);
    });
    connect(scene, &FlowScene::nodeContextMenu, [scene,this](QtNodes::Node & node, const QPointF& pos){
        ModuleLibrary::updateVarName(*scene, node, this);
    });
    //3:启用工具栏、展示选择项目窗口
    ui->statusbar->showMessage("节点模型数据加载已完成");
    ui->tw_toolbar->setEnabled(true);
    ui->tw_node->setEnabled(true);
    ui->menubar->setEnabled(true);
    rProjectDialog->show();
}

/// 只负责注册右键菜单，并返回右键菜单的数据模型,注册的数据包括普通模块、子系统模块
/// 该函数在项目打开完毕后执行，每次打开项目都要重新注册一次,或每次增加了新的子系统都要重新注册一次
std::shared_ptr<DataModelRegistry> MainWindow::registerDataModels(){

    std::list<Invocable> parserResult = _moduleLibrary->getParseResult();
    auto ret = std::make_shared<DataModelRegistry>();

    //此处在单独的线程中，不能使用默认主线程的数据库连接
    //    QSqlDatabase sqlite;
    //    sqlite.addDatabase("QSQLITE");
    //    sqlite.setDatabaseName(QApplication::applicationDirPath()+"/sqlite/node.db3");
    //    sqlite.open();
    AICCSqlite sqlite;


    //注册所有已有的模块
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

    //注册所有的子系统
    for (const auto &inv: _subsystemLibrary->getInvocableList()) {
        auto f = [inv]() {
            std::unique_ptr<InvocableDataModel> p = std::make_unique<InvocableDataModel>(inv);
            p->setCaption(p->name());
            return p;
        };
        ret->registerModel<InvocableDataModel>(f, "自定义模块");
    }

    return ret;
}

///初始化导入脚本对话框的内容
void MainWindow::initImportScriptDialog(){
    //选择文本后响应函数
    connect(isDialog,&ImportScriptDialog::packageSelected,this,[&](const QString packFile){

        //        QtConcurrent::run([&](){
        _moduleLibrary->addPackage(QString(packFile).toStdString());
        //        });

        //          _moduleLibrary->addPackage(QString(packFile).toStdString());

        //        QtConcurrent::run([&pf,&ml](){
        ///旧的单个文件导入
        //1:解析选择文件中的node
        //            _moduleLibrary->importFiles(files);
        //            std::list<Invocable> parserResult = _moduleLibrary->getParseResult();
        //此处只通知initNodeEditor函数中链接的registrySceneGenerateNodeMenu函数执行后续操作即可
        //            emit scriptParserCompleted(parserResult);
        //            qDebug() << "--------------module library instance" << ml;
        //                    qDebug() << "-------------module library instance" << pf;
        ///新方式，以包的方式导入
        //            ml->addPackage(QString(pf).toStdString());
        //        });
    });

    connect(_moduleLibrary,&ModuleLibrary::fileParserCompleted,this,[&](int count, int index){
        qDebug() << "index:" <<index << "   count:" << count;
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





