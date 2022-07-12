#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    npDialog(new NodeParametersDialog(parent)),
    npmilDialog ( new NodeParametersMILDialog(parent)),
    diDialog ( new DataInspectorDialog(parent)),
    eDialog ( new EditorWindow(parent)),
    //    _process (QSharedPointer<AICCProcess>(new AICCProcess)),
    _currentProjectDataModel(QSharedPointer<ProjectDataModel>(new ProjectDataModel)),
    _recentProjectDataModel(QSharedPointer<RecentProjectDataModel>(new RecentProjectDataModel)),
    _categoryDataModel(QSharedPointer<CategoryDataModel>(new CategoryDataModel))

{
    //    sqlite.initDatabaseConnection();
    //    _moduleLibrary = QSharedPointer<ModuleLibrary>(new ModuleLibrary());
    ui->setupUi(this);

    this->setWindowState(Qt::WindowMaximized);
    this->setAttribute(Qt::WA_QuitOnClose);
    rProjectDialog=new RecentProjectDialog(_recentProjectDataModel,parent);
    projectDialog=new ProjectDialog(_currentProjectDataModel,_recentProjectDataModel,parent);
    isDialog =new ImportScriptDialog(_currentProjectDataModel,parent);
    emDialog = new ExportModuleDialog(_currentProjectDataModel,parent);
    nodeTreeDialog = new NodeTreeDialog(_categoryDataModel,_currentProjectDataModel,parent);

    QVector<QWidget*> vw;
    vw.append(ui->cb_select_platform);
    vw.append(ui->tb_script_deploy);
    vw.append(ui->tb_run);
    vw.append(ui->tb_stop);
    _process = QSharedPointer<AICCProcess>(new AICCProcess(vw));

    this->initProcess();
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
    this->initRecentProjectDialog();
    this->initProjectDialog();
    this->initProjectDataModel();

    //    std::cout << "std::cout";
    //    printf("printf");
    //    qDebug() << "qDebug";


}

MainWindow::~MainWindow()
{
    //    sqlite.closeConnection();
    delete ui;
}

///用于处理consoel内容重定向
void MainWindow::showMsg(QString msg){
    ui->pte_output->appendPlainText(msg);
}

/**
 * @brief MainWindow::initProcess   初始化process
 */
void MainWindow::initProcess(){
    //获得Process的标准输出
    connect(_process.get(),&QProcess::readyRead,this,[&](){
        ui->pte_output->appendPlainText(_process->readAll());
    });

    //process执行完成
    connect(_process.get(),&AICCProcess::tasksCompleted,this,[&](const bool success,const QString msg){
        //        ui->tb_script_deploy->setEnabled(true);
        //        ui->tb_run->setEnabled(true);
        //        ui->tb_stop->setEnabled(true);
    });

}

///初始化菜单
void MainWindow::initMenu()
{
    connect(ui->actionExit,&QAction::triggered,this,&QWidget::close);
    connect(ui->actionNodeWindow,&QAction::toggled,ui->dw_left,&QDockWidget::setVisible);
    connect(ui->actionPropertyWindow,&QAction::toggled,ui->dw_right,&QDockWidget::setVisible);
    connect(ui->actionAbout,&QAction::triggered,this,&QApplication::aboutQt);
    connect(ui->actionNewProject,&QAction::triggered,projectDialog,&ProjectDialog::show);
    connect(ui->actionOpen,&QAction::triggered,this,&MainWindow::openProjectAction);
    connect(ui->actionSave,&QAction::triggered,this,&MainWindow::saveProjectAction);
    //        connect(ui->actionCreateSubSystem,&QAction::triggered,this,[&]{_moduleLibrary->newSubsystem(this);});
}

///初始化左侧子系统关系树
void MainWindow::initTreeView()
{
    //当双击树形菜单节点后打开子窗口
    connect(ui->tw_node,&AICCStructTreeWidget::treeNodeDoubleClicked,this,[&](QTreeWidgetItem *item){
        //包名类名由data带入，权限为UserRole
        QString pn = item->data(0,Qt::ItemDataRole::UserRole).toString();
        QStringList sl = pn.split("::");
        //当拆解出的字符串包含包名与类名打开子窗口
        if(sl.size()==2){
            //            _moduleLibrary->openSubsystem(this,sl.at(0).toStdString(),sl.at(1).toStdString());
            openSubsystem(sl.at(0).toStdString(),sl.at(1).toStdString());

        }
    });

    //初始化过滤器文本框
    connect(ui->le_tree_filter,&QLineEdit::textChanged,[&](const QString &text){
        ui->tw_node->filterTreeWidgetItem(text);
    });

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
    ui->pb_print->setVisible(false);
    //    ui->tb_import_code->setVisible(false);
    ui->pb_script_generator->setVisible(false);             //代码生成
    ui->tb_code_compiler->setVisible(false);                //代码构建
    ui->tb_edit_script->setVisible(false);                  //编写脚本

    //隐藏掉上方dock的标题栏
    QWidget *titleBarWidget = ui->dw_toolbar->titleBarWidget();
    QWidget *nullTitleBarWidget = new QWidget();
    ui->dw_toolbar->setTitleBarWidget(nullTitleBarWidget);
    delete titleBarWidget;
    ui->tw_toolbar->setCurrentIndex(0);
    ui->tw_bottom->setCurrentIndex(0);

    //设置New按钮显示项目子窗口
    connect(ui->tb_new,&QToolButton::clicked,projectDialog,&ProjectDialog::show);

    //显示node模块的窗口
    connect(ui->pb_library_browser,&QPushButton::clicked,this,[&]{
        nodeTreeDialog->show();
    });

    //生成代码按钮
    connect(ui->pb_script_generator,&QPushButton::clicked,this,[&](){
        generateCode();

    },Qt::UniqueConnection);

    //导出子系统模块按钮
    connect(ui->tb_export_module,&QToolButton::clicked,this,[&]{

        emDialog->show();
    });

    //导入code脚本按钮
    connect(ui->tb_import_code,&QPushButton::clicked,this,[&]{
        //此处代码为显示单独得窗口来进行导入
        //        isDialog->show();

        //该代码为点击导入按钮后直接进行导入
        QString srcPath = QFileDialog::getExistingDirectory(this,"请选择功能模块包",QApplication::applicationDirPath());
        if(!srcPath.isEmpty()){
            QStringList pathList = srcPath.split("/");                              //拆分目录
            const QString lastDirectory = pathList.at(pathList.size()-1);                 //最后的目录为导入的模块包名
            const QString prefix_path = "Custom/"+lastDirectory;
            const QString &dstPath = QApplication::applicationDirPath()+"/ICVOS/Custom/"+lastDirectory+"/";

            //0:此处可检查导入得包格式结构是否正确
            qInfo() << "检查导入包格式是否正确";
            QDir srcDir(srcPath);
            QFileInfoList srcFileInfoList = srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
            if(srcFileInfoList.size()!=3){
                qWarning() << "导入目录结构数量不正确:" << srcFileInfoList.size() << " " << srcPath;
                return;
            }
            foreach(auto srcFileInfo,srcFileInfoList){
                if(srcFileInfo.baseName()!="include" && srcFileInfo.baseName()!="lib" && srcFileInfo.baseName()!="share"){
                    qWarning() << "导入目录内容名称不正确:" << srcFileInfo.baseName();
                    return;
                }
            }

            //0.5:判断QApplication::applicationDirPath()+"/ICVOS/Custom/" 是否存在，如不存在创建该目录
            const QString customPath = QApplication::applicationDirPath()+"/ICVOS/Custom";
            QDir customDir(customPath);
            if(!customDir.exists()){
                customDir.mkdir(customPath);
            }

            //1:判断目录是否存在，如目录存在不允许拷贝
            qInfo() << "检查导入位置是否存在相同包";
            QDir dstDir(dstPath);
            if(dstDir.exists()){
                qWarning() << "导入失败,导入代码模块已存在：" << lastDirectory ;
                return;
            }

            //2:拷贝文件夹到ICVOS/Custom目录下
            qInfo() << "拷贝模块包到目标路径";
            copyDirectory(srcPath,dstPath,false);

            //3:addPackage增加，在addPackage完成后得事件中完成4、5两步
            qInfo() << "导入新增模块包到系统";
            this->setEnabled(false);
            try{
                _moduleLibrary->addPackage(QString(dstPath).toStdString());
            } catch (const std::exception &e ){
                qCritical() << "module library exception:" << e.what();
            }

            this->setEnabled(true);

            //4:监测重新导入的内容，如发现有未导入的模块则写到数据库中
            qInfo() << "模块包数据写入数据库";
            QVector<QString> importClassName = importWithoutModuleSubsystem(_moduleLibrary->getInvocableList());

            //5:将导入得包写入adas-packages.json中
            QString adas_packages_file = QApplication::applicationDirPath()+"/ICVOS/adas-packages.json";
            QFile file(adas_packages_file);
            if(file.open(QIODevice::ReadWrite)){
                QByteArray data = file.readAll();
                QJsonDocument adas_packages_doc = QJsonDocument::fromJson(data);
                if(!adas_packages_doc.isObject()){
                    qWarning() << adas_packages_file << " 读取失败,文件不是json格式" << endl;
                    return;
                }
                QJsonObject adas_packages_json = adas_packages_doc.object();
                QJsonArray prefix_paths = adas_packages_json["prefix_paths"].toArray();
                if(prefix_paths.contains(QJsonValue::fromVariant(prefix_path))){
                    qWarning() << "配置文件adas_package.json中已包含" << prefix_path << " 无需再次导入";
                    return;
                }else{
                    prefix_paths.append(prefix_path);
                    adas_packages_json["prefix_paths"] = prefix_paths;
                    adas_packages_doc.setObject(adas_packages_json);
                    file.close();
                    file.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate);
                    file.write(adas_packages_doc.toJson(QJsonDocument::Indented));
                    file.close();
                    qInfo() << "导入" << prefix_path << "完成";
                }

            }else{
                qWarning() << "打开ICVOX/adas-package.json文件失败" << endl;

            }

        }
    });

    //导入子系统模块按钮
    //导入一个flow文件成为自定义模块
    connect(ui->tb_import_module,&QToolButton::clicked,this,[&]{
        QString importModuleName = QFileDialog::getOpenFileName(this,tr("请选择要导入的自定义模块文件"),QApplication::applicationDirPath(),tr("自定义模块flow文件 (*.flow)"),Q_NULLPTR,QFileDialog::ReadOnly);
        QFileInfo imFileInfo(importModuleName);
        QString imPath =  imFileInfo.absolutePath();
        imPath.replace("\\", "/");
        //        qDebug() << "imPath: " << imPath.split("/").at(imPath.split("/").count()-1);
        QString imPackage = imPath.split("/").at(imPath.split("/").count()-1);

        //0：在ICVOS/Function/Component下创建模块包
        //        QDir packageDir(QApplication::applicationDirPath()+"/ICVOS/Function/Component/"+imPackage);
        QDir packageDir(this->_currentProjectDataModel->projectSubSystemPath()+"/"+imPackage);
        if (!packageDir.exists()) {
            if (!packageDir.mkdir(packageDir.absolutePath())){
                qWarning() << "创建路径失败:" << packageDir.absolutePath();
                return false;
            }
        }else{
            //如果文件存在则不允许导入
            QFileInfo objectFileInfo(packageDir.path()+"/"+imFileInfo.fileName());
            if(objectFileInfo.exists()){
                qWarning() << "导入失败,导入组合模块已存在：" << imPackage+"/"+imFileInfo.fileName();
                return false;
            }
        }

        //1：拷贝模块包到指定路径，如果拷贝成功则进行_moduleLibrary->importCompleted()导入
        if(copyFile(importModuleName,packageDir.path()+"/"+imFileInfo.fileName(),true)){
            //更新主Scene的注册数据
            ui->sw_flowscene->getCurrentView()->scene()->setRegistry(_moduleLibrary->test2());
            //更新分类数据中的注册数据
            _categoryDataModel->refreshCategoryDataModel(*_moduleLibrary);;
            //2：加入信息到数据库中,导入的子系统模块全部放倒自定义模块下，自定义模块的pid为3
            importWithoutModuleSubsystem(_moduleLibrary->subsystemLibrary().getInvocableList());

            qInfo() << "导入组合模块完成";
        }else{
            qWarning() << "组合模块：" << importModuleName << " 不能拷贝到指定位置：" << packageDir.path()+"/"+imFileInfo.fileName();
            return false;
        }

        return true;
    });

    ///打开按钮响应动作
    connect(ui->pb_open,&QPushButton::clicked,this,&MainWindow::openProjectAction);

    ///保存按钮响应动作，当前只保存一个NodeEditor的内容，子系统实现后需要保存多个NodeEditor内容
    connect(ui->pb_save,&QPushButton::clicked,this,&MainWindow::saveProjectAction);

    ///点击显示数据检查器窗口
    connect(ui->pb_dataInspector,&QPushButton::clicked,this,[&]{
        QJsonObject jo = getConfig();
        monitorDialog = new MonitorDialog(this);
        monitorDialog->show();
    });

    ///平台选择下拉框
    connect(ui->cb_select_platform,&QComboBox::currentTextChanged,this,[&](const QString &text){
        //        if(text==QString::fromLocal8Bit("SelectPlatform") ){
        if(ui->cb_select_platform->currentIndex()==0){
            ui->tb_code_compiler->setEnabled(false);
            ui->tb_script_deploy->setEnabled(false);
            ui->tb_run->setEnabled(false);
            ui->tb_stop->setEnabled(false);
        }
        else{
            ui->tb_code_compiler->setEnabled(true);
            ui->tb_script_deploy->setEnabled(true);
            ui->tb_run->setEnabled(true);
            ui->tb_stop->setEnabled(true);
        }
    });

    ///显示脚本编辑器
    connect(ui->tb_edit_script,&QToolButton::clicked,this,[&]{
        eDialog->show();
    });


    ///代码编译按钮code compiler
    connect(ui->tb_code_compiler,&QToolButton::clicked, this,[&](){
        QVector<QString> v;
        v << "build_bst.sh" << "build_jetson.sh" << "build_mdc.sh" << "build_x86_64.sh";
        _process->processStart(v,ui->cb_select_platform->currentIndex());
    });


    ///代码部署按钮deploy
    connect(ui->tb_script_deploy,&QToolButton::clicked,this,[&](){
        //单独执行部署脚本代码
        //        QVector<QString> v;
        //        v << "deploy_bst.sh" << "deploy_jetson.sh" << "deploy_mdc.sh" << "deploy_x86_64.sh";
        //        if(ui->cb_select_platform->currentIndex()>0){
        //            _process->processStart(v,ui->cb_select_platform->currentIndex());
        //        }

        //执行 生成代码、编译代码、部署代码
        //1:生成代码
        qInfo() << "执行生成代码";

        generateCode();

        //2：执行 编译代码、部署代码 多任务
        QVector<QString> v_build,v_deploy,v;
        v_build << "build_bst.sh" << "build_jetson.sh" << "build_mdc.sh" << "build_x86_64.sh";
        v_deploy << "deploy_bst.sh" << "deploy_jetson.sh" << "deploy_mdc.sh" << "deploy_x86_64.sh";

        v << v_build.at(ui->cb_select_platform->currentIndex()-1) << v_deploy.at(ui->cb_select_platform->currentIndex()-1);

        _process->mulProcessStart(v);

    });




    ///run
    connect(ui->tb_run,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "run_bst.sh" << "run_jetson.sh" << "run_mdc.sh" << "run_x86_64.sh";
        if(ui->cb_select_platform->currentIndex()>0)
            _process->processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///stop
    connect(ui->tb_stop,&QToolButton::clicked,this,[&](){
        QVector<QString> v;
        v << "stop_bst.sh" << "stop_jetson.sh" << "stop_mdc.sh" << "stop_x86_64.sh";
        if(ui->cb_select_platform->currentIndex()>0)
            _process->processStart(v,ui->cb_select_platform->currentIndex());
    });

    ///在线标定按钮OnlineCalibration->Calibration
    connect(ui->tb_calibration,&QToolButton::clicked,this,[&](){
        QJsonObject jo = getConfig();
        cDialog = new CalibrationDialog(jo.value("deviceIP").toString(),_currentProjectDataModel,this);
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
        //        _moduleLibrary->newSubsystem(this);


        NewSubsystemDialog dialog(this);

        dialog.setCategoryComboBox(_categoryDataModel->currentUseCategoryFullPath());
        NewSubsystemDialog::SubsystemNameType subsystem_name;
        for (;;) {
            if (!dialog.exec())
                return;
            subsystem_name = dialog.getSubsystemName();
            if (subsystem_name.first.empty() || subsystem_name.second.empty()) {
                QMessageBox::critical(this, "错误", "包名或子系统名不能为空");
                continue;
            }

            //            if (_subsystemLibrary.hasSubsystem(subsystem_name.first, subsystem_name.second)) {
            if(_moduleLibrary->subsystemLibrary().hasSubsystem(subsystem_name.first,subsystem_name.second)){
                QMessageBox::critical(this, "错误", "子系统已经存在");
                continue;
            }
            break;
        }
        _moduleLibrary->subsystemLibrary().newSubsystem(subsystem_name.first, subsystem_name.second);
        emit _moduleLibrary->importCompleted();

        //    openSubsystem(parent, subsystem_name.first, subsystem_name.second);
        //        openSubsystem(parent,dialog.getSubsystemDataModel());
        //        SubsystemDataModel &subsystemDataModel = dialog.getSubsystemDataModel();
        std::map<std::string,std::string> subsystemDataModel = dialog.getSubsystemDataModel();
        const std::string package = subsystemDataModel.at("package");
        const std::string name = subsystemDataModel.at("name");
        auto subsystemWindow = new SubsystemWindow(_moduleLibrary.get(), _moduleLibrary->subsystemLibrary().getSubsystem(package, name), this);
        //当子系统有node创建或删除时，将信号继续传送到外部
        connect(subsystemWindow,&SubsystemWindow::subsystemCreatedOrDeleted,this,[&]{
            emit subsystemWindow->subsystemCreatedOrDeleted();
        });
        subsystemWindow->show();
        subsystemWindow->setBusinessData(subsystemDataModel);

    });

    ///测试dialog显示
    //    connect(ui->pb_modelSettings,&QPushButton::clicked,this,[&](){
    //        TestDialog *tdialog = new TestDialog(this);
    //        tdialog->show();
    //    });
}

void MainWindow::generateCode(){
    //generate cpp code
    AICCFlowScene *scene = ui->sw_flowscene->getCurrentView()->scene();

    //不放到项目路径中，放到平台执行目录中，否则编译时找不到
    std::string generatePath = (QApplication::applicationDirPath()+"/App").toStdString();
    std::filesystem::path dir(generatePath);
    SourceGenerator::generateCMakeProject(dir,*scene,*_moduleLibrary);

    //generate shell script
    SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/ICVOS/adas-target-jetson.json").toStdString(),"jetson",*scene,_moduleLibrary->packageLibrary());
    SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/ICVOS/adas-target-bst.json").toStdString(),"bst",*scene,_moduleLibrary->packageLibrary());
    SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/ICVOS/adas-target-mdc.json").toStdString(),"mdc",*scene,_moduleLibrary->packageLibrary());
    SourceGenerator::generateScript(dir,QApplication::applicationDirPath().append("/ICVOS/adas-target-x86_64.json").toStdString(),"x86_64",*scene,_moduleLibrary->packageLibrary());

    generatePath.append("/generate.cpp");
    //    eDialog->openTextFile(QString::fromStdString(generatePath));
    //    eDialog->show();
}

///初始化面包屑导航
void MainWindow::initBreadcrumbNavigation(){
    QStringList lpath;
    lpath << "根目录";
    ui->l_breadcrumb_navigation->makeNavigationData(lpath);
    ui->l_breadcrumb_navigation->refreshNavigationView();

    //暂时隐藏该控件
    ui->l_breadcrumb_navigation->hide();

    ///相应点击链接的操作
    connect(ui->l_breadcrumb_navigation,&AICCBreadcrumbNavigation::linkActivated,this,[&](const QString &link){
        ui->sw_flowscene->setCurrPagePathName(link);
        qDebug() << "clicked link:" << link ;
    });
}

///初始化StackedWidget
void MainWindow::initStackedWidget(){

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
        auto invocableDataModel = static_cast<InvocableDataModel*>(nodeDataModel);
        const auto & invocable = invocableDataModel->invocable();
        if(invocable.getType() == Invocable::Subsystem){
            //            _moduleLibrary->openSubsystem(this, invocable.getPackage(), invocable.getSubsystemName());
            openSubsystem(invocable.getPackage(),invocable.getSubsystemName());

        }
        //如果点击的是MIL模块，显示特殊的文件加载子窗口
        else if(nodeDataModel->name()=="business_package::mil_import"){
            npmilDialog->show();
        }
        else {
            npDialog->show();
            QTableWidget *nptw =  npDialog->getTableNodeParameters();
            //填充属性窗口的表格
            nptw->setRowCount(0);
            if(nodeDataModel==Q_NULLPTR) return;
            nptw->setRowCount(nptw->rowCount()+1);
            nptw->setItem(0,0,new QTableWidgetItem("caption"));
            nptw->setItem(0,1,new QTableWidgetItem(nodeDataModel->caption()));

            nptw->setRowCount(nptw->rowCount()+1);
            nptw->setItem(1,0,new QTableWidgetItem("name"));
            nptw->setItem(1,1,new QTableWidgetItem(nodeDataModel->name()));

            qDebug() << "emit getNodeDataModel";
        }
    });


    ///向StackedWidget控件中增加第一个页面，并增加第一个FlowScene
    ui->sw_flowscene->addNewPageFlowScene("");


}

///保存项目动作
void MainWindow::saveProjectAction(){
    //0：判断当前是否为未关联项目，如未关联项目要求用户先创建项目
    if(_currentProjectDataModel->projectName()==""){
        int result  = projectDialog->exec();
        if(result==QDialog::Rejected) return;
    }

    //2：保存当前内容到flow文件中
    AICCFlowView *fv = static_cast<AICCFlowView *>(ui->sw_flowscene->currentWidget());
    qDebug() << _currentProjectDataModel->projectPath()<< "   "<< _currentProjectDataModel->projectName();
    if(_currentProjectDataModel->projectPath()=="" || _currentProjectDataModel->projectName()==""){
        QMessageBox::critical(Q_NULLPTR,"critical","请先选择项目再进行保存",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    for(const QString &ssf :_currentProjectDataModel->flowSceneSaveFiles()){
        QString saveFileName = _currentProjectDataModel->projectPath()+"/"+ssf;
        qInfo() << "save file name:" << saveFileName;
        QFile file(saveFileName);
        if(file.open(QIODevice::WriteOnly)){
            file.write(fv->scene()->saveToMemory());
        }
    }
}

///打开项目动作
bool MainWindow::openProjectAction(){
    QString fileName = QFileDialog::getOpenFileName(this,tr("打开项目"),QDir::homePath(),tr("项目 (*.xml)"));
    QFileInfo fileInfo(fileName);
    if(!fileInfo.exists(fileName)) return false;
    QString projectProject = fileInfo.absoluteFilePath().split("/project.xml")[0];
    _currentProjectDataModel->setProject(QFileInfo(projectProject));
    return true;
}

///打开项目完成后执行部分
void MainWindow::projectDataModelLoadCompletedAction(const QString pname,const QString ppath){
    //0:数据准备
    this->setWindowTitle(_currentProjectDataModel->projectName()+" ("+_currentProjectDataModel->projectPath()+") "+" - 图形化ADAS/AD应用开发系");
    ui->sw_flowscene->initDefaultScenes();
    FlowScene *scene =  static_cast<AICCFlowView *>(ui->sw_flowscene->currentWidget())->scene();


    //1:判断是否有subsystem文件夹如果有则直接设置目录，否则创新的目录再进行设置
    QString psubsystem = _currentProjectDataModel->projectSubSystemPath();
    QDir dirSubsystem;
    if(!dirSubsystem.exists(psubsystem)) dirSubsystem.mkpath(psubsystem);
    std::string sp = _currentProjectDataModel->projectSubSystemPath().toStdString();
    _moduleLibrary->setSubsystemPath(sp);


    //设置项目的subsystem目录与系统级的subsystem
    //    std::string otherSubsystemPath = QString(QApplication::applicationDirPath()+"/ICVOS/Function/Component").toStdString();
    //    _moduleLibrary->subsystemLibrary()->setSystemPath(otherSubsystemPath);

    //2:注册所有的功能模块到右键上
    std::shared_ptr<DataModelRegistry> dmr = registerDataModels();
    ui->sw_flowscene->getCurrentView()->scene()->setRegistry(dmr);

    //3：将名称为mainFlowScene的文件内容加载到主FlowScene上
    QString loadFileName = _currentProjectDataModel->projectPath()+"/"+ _currentProjectDataModel->flowSceneSaveFiles()[0];
    if (!QFileInfo::exists(loadFileName)) return;

    QFile loadFile(loadFileName);
    if(!loadFile.open(QIODevice::ReadOnly)) return;

    scene->clearScene();
    QByteArray wholeFile = loadFile.readAll();
    scene->loadFromMemory(wholeFile);

    //3.5加载左侧结构树内容
    ui->tw_node->fillInitLeftTreeData(*_moduleLibrary,_currentProjectDataModel->projectName(),scene);

    //4:将当前打开的项目排序到第一位
    _recentProjectDataModel->sortProjectFirst(_currentProjectDataModel->projectName(),_currentProjectDataModel);

    //5:打开新项目后分类数据要刷新
    _categoryDataModel->refreshCategoryDataModel(*_moduleLibrary);

}

///初始化最近打开项目窗口
void MainWindow::initRecentProjectDialog(){
    connect(rProjectDialog,&RecentProjectDialog::setCurrentProjectDataModelCompleted,this,[&](QSharedPointer<ProjectDataModel> pdm){
        _currentProjectDataModel->setProject(pdm->projectName(),pdm->projectPath());
    });
    connect(rProjectDialog,&RecentProjectDialog::recentProjectDialogClosed,this,[&]{
        //        qDebug() <<" close ";
        forceClose = true;
        this->close();

    });
    //最近项目窗口点击
    connect(rProjectDialog,&RecentProjectDialog::newProjectTriggered,this,[&]{
        //        projectDialog->show();
        projectDialog->showFromRecentProjectDialog();
        rProjectDialog->hide();
    });
    connect(rProjectDialog,&RecentProjectDialog::openLocalProject,this,[&](){
        if(this->openProjectAction())
            rProjectDialog->hide();
    });
}


///初始化数据模型
void MainWindow::initProjectDataModel(){
    connect(_currentProjectDataModel.get(),&ProjectDataModel::projectDataModelLoadCompleted,this,&MainWindow::projectDataModelLoadCompletedAction);

}

void MainWindow::initProjectDialog(){
    connect(projectDialog,&ProjectDialog::projectDialogCanceled,this,[&](bool showRecentProjectDialog){
        if(showRecentProjectDialog)
            rProjectDialog->show();
    });
}

///NodeEditor数据处理部分
//初始化时初始化主Scene的右键菜单，和NodeTreeDialog的node分类数据
//TODO:
void MainWindow::initNodeEditor(){
    std::string systemSubsystemPath = QString(QApplication::applicationDirPath()+"/ICVOS/Function/Component").toStdString();
    _moduleLibrary = QSharedPointer<ModuleLibrary>(new ModuleLibrary);
    _moduleLibrary->setSystemSubsystemPath(systemSubsystemPath);

    //消息发送位置ModuleLibrary::addPackage函数
    connect(_moduleLibrary.get(),&ModuleLibrary::parsingStep,this,[&](std::string package_name){
        qInfo() << "正在载入包:" << QString::fromStdString(package_name) ;
        QApplication::processEvents(QEventLoop::AllEvents, 33);
    });

    //接收解析文件发生错误的信号
    connect(_moduleLibrary.get(),&ModuleLibrary::errorOccured,this,[&](const QString &error_message){
        qCritical() << "模块加载错误:" << error_message;
        QMessageBox mb(this);
        mb.setWindowTitle("模块加载错误");
        mb.setText(error_message);
        mb.move((QApplication::desktop()->width()-mb.widthMM())/2,(QApplication::desktop()->height()-mb.heightMM())/2);
        mb.exec();
        //        mb.critical(this,"加载错误",error_message);

        this->forceClose = true;
        this->close();
    });

    //1:解析pakage文件
    const QString path = QApplication::applicationDirPath()+"/ICVOS/";
    QStringList files = getADASPackagesFileList(path);

    //2:执行加载前的准备动作emit _moduleLibrary->importCompleted();
    ui->statusbar->showMessage("正在加载节点模块数据");
    ui->tw_toolbar->setEnabled(false);
    ui->tw_node->setEnabled(false);
    ui->menubar->setEnabled(false);

    //    ui->pte_output->appendPlainText("--------------------");
    //3:创建单独线程，耗时操作放到其中，防止界面卡死
    QtConcurrent::run([&,files](){
        //        try{
        _moduleLibrary->importFiles(files);
        //        }catch(const std::exception &e){
        //            QMessageBox::critical(this,"加载模块错误","系统加载ICVOS资源出错，请检查./ICVOS/adas-packages.json内容是否正确");
        //            qCritical() << "module library exception:" << e.what();
        //        }

        std::list<Invocable> parserResult = _moduleLibrary->getParseResult();
        emit scriptParserCompleted(parserResult);
    });
    qRegisterMetaType<std::list<Invocable>>("std::list<Invocable>");

    //4:包数据解析完成后续工作
    connect(this,&MainWindow::scriptParserCompleted,this,&MainWindow::scriptParserCompletedAction);

    //5:响应importCompleted
    connect(_moduleLibrary.get(),&ModuleLibrary::importCompleted,this,[&](){
        //        ui->pte_output->appendPlainText("模型数据加载完毕");
        ui->sw_flowscene->getCurrentView()->scene()->setRegistry(this->registerDataModels());
        _categoryDataModel->refreshCategoryDataModel(*_moduleLibrary);
    });

    //6:当主界面内容加载完成，连接节点的创建、删除信号
    connect(ui->sw_flowscene->getCurrentView()->scene(),&AICCFlowScene::sceneLoadFromMemoryCompleted,this,&MainWindow::sceneLoadFromMemoryCompletedAction);

    //7:子系统节点创建或删除时刷新左侧节点
    connect(_moduleLibrary.get(),&ModuleLibrary::subsystemCreatedOrDeleted,this,[&](){
        ui->tw_node->fillInitLeftTreeData(*_moduleLibrary,_currentProjectDataModel->projectName(),ui->sw_flowscene->getCurrentView()->scene());
    });

    //8:addPackage响应函数，addPackage完成后可在此执行后续操作
    connect(_moduleLibrary.get(),&ModuleLibrary::fileParserCompleted,this,[&](int count, int index){

    });


    //9:scene放置或删除node时判断该node是否为subsystem,如果未subsystem则刷新左侧结构树
    auto refreshSubsystemTree = [&](Node &n){
        const auto *model = static_cast<const InvocableDataModel*>(n.nodeDataModel());
        if(model->invocable().getType()==Invocable::Subsystem){
            ui->tw_node->fillInitLeftTreeData(*_moduleLibrary,_currentProjectDataModel->projectName(),ui->sw_flowscene->getCurrentView()->scene());
        }
    };
    connect(ui->sw_flowscene->getCurrentView()->scene(),&AICCFlowScene::nodeCreated,this,refreshSubsystemTree);
    connect(ui->sw_flowscene->getCurrentView()->scene(),&AICCFlowScene::nodeDeleted,this,refreshSubsystemTree);

}

static Invocable::Type invocableType = Invocable::Class;
///主场景加载完成后初始化连接node的创建、删除信号
void MainWindow::sceneLoadFromMemoryCompletedAction(bool isCompleted){
    if(isCompleted){
        AICCFlowScene * scene = ui->sw_flowscene->getCurrentView()->scene();
        connect(scene,&AICCFlowScene::nodeCreated,this,[&](Node &node){
            const auto *model = static_cast<InvocableDataModel*>(node.nodeDataModel());
            const auto & invocable = model->invocable();
            if(invocable.getType()==Invocable::Subsystem){
                qDebug() << "create";
                ui->tw_node->fillInitLeftTreeData(*_moduleLibrary,_currentProjectDataModel->projectName(),ui->sw_flowscene->getCurrentView()->scene());
            }
        });

        //在响应节点删除后信号前先判断该节点是什么类型
        connect(scene,&AICCFlowScene::nodeDeleted,this,[&](Node &node){
            invocableType = static_cast<InvocableDataModel*>(node.nodeDataModel())->invocable().getType();
        });

        connect(scene,&AICCFlowScene::afterNodeDeleted,this,[&](){
            //判断当前记录的invocableType类型为Subsysem才执行刷新操作
            if(invocableType==Invocable::Subsystem)
                ui->tw_node->fillInitLeftTreeData(*_moduleLibrary,_currentProjectDataModel->projectName(),ui->sw_flowscene->getCurrentView()->scene());
        });
    }
}

///导入完成响应动作，此处不要用lambda表达式，会导致跨线程调用问题
void MainWindow::importCompletedAction(){
    ui->sw_flowscene->getCurrentView()->scene()->setRegistry(this->registerDataModels());
}

///包数据解析完毕工作
void MainWindow::scriptParserCompletedAction(std::list<Invocable> parserResult){

    //1:生成NodeTreeDialog的菜单结构
    _categoryDataModel->refreshCategoryDataModel(*_moduleLibrary);
    //2:初始化模块变量相关操作
    AICCFlowScene *scene = ui->sw_flowscene->getCurrentView()->scene();
    connect(scene, &FlowScene::nodeCreated, [scene](QtNodes::Node & node){
//        ModuleLibrary::generateVarNameIfEmpty(*scene, node);
        scene->generateVarNameIfEmpty(*scene,node);
    });

    //3:重命名变量
    connect(ui->sw_flowscene->getCurrentView(),&AICCFlowView::nodeRename,this,[scene,this](Node &node){
//        ModuleLibrary::updateVarName(*scene,node,this);
        scene->updateVarName(*scene,node,this);
    });

    //    connect(scene, &FlowScene::nodeContextMenu, [scene,this](QtNodes::Node & node, const QPointF& pos){
    //        ModuleLibrary::updateVarName(*scene, node, this);
    //    });
    //3:启用工具栏、展示选择项目窗口
    ui->statusbar->showMessage("节点模型数据加载已完成");
    ui->tw_toolbar->setEnabled(true);
    ui->tw_node->setEnabled(true);
    ui->menubar->setEnabled(true);
    rProjectDialog->show();

}

/**
 * @brief MainWindow::registerDataModels    只负责注册右键菜单，并返回右键菜单的数据模型,注册的数据包括普通模块、子系统模块
 *                                          该函数在项目打开完毕后执行，每次打开项目都要重新注册一次,或每次增加了新的子系统都要重新注册一次
 * @return
 */
std::shared_ptr<DataModelRegistry> MainWindow::registerDataModels(){

    std::list<Invocable> parserResult = _moduleLibrary->getParseResult();
    auto ret = std::make_shared<DataModelRegistry>();


    //注册系统默认模块
    for(auto it = parserResult.begin();it!=parserResult.end();++it){
        const auto &inv = *it;
        QString nodeName = QString::fromStdString(inv.getName());
        QString category = QString::fromStdString(_categoryDataModel->makeCategoryFullPath(nodeName).second);
        //获得caption
        QString caption = getCaptionByName(nodeName);

        auto f = [inv,nodeName,caption](){
            std::unique_ptr<InvocableDataModel> p = std::make_unique<InvocableDataModel>(inv);
            p->setCaption(caption);
            return p;
        };
        if(category!="")
            ret->registerModel<InvocableDataModel>(f,category);
        else
            ret->registerModel<InvocableDataModel>(f,"其他");
    }

    //注册所有的子系统
    for (const auto &inv: _moduleLibrary->subsystemLibrary().getInvocableList()) {
        QString nodeName = QString::fromStdString(inv.getName());
        QString category = QString::fromStdString(_categoryDataModel->makeCategoryFullPath(nodeName).second);
        //获得caption
        QString caption = getCaptionByName(nodeName);

        auto f = [inv,caption]() {
            std::unique_ptr<InvocableDataModel> p = std::make_unique<InvocableDataModel>(inv);
            p->setCaption(caption);
            return p;
        };
        if(category!="")
            ret->registerModel<InvocableDataModel>(f,category);
        else
            ret->registerModel<InvocableDataModel>(f, "自定义模块");
    }

    return ret;
}

/**
 * @brief MainWindow::closeEvent    窗口关闭确认
 * @param e
 */
void MainWindow::closeEvent(QCloseEvent *e){
    //如果强制关闭标识位为ture直接关闭窗口，不弹出提示窗口
    if(this->forceClose) {
        e->accept();
        return;
    }

    if(0 == QMessageBox::warning(this,QStringLiteral("退出"),QStringLiteral("请确认项目已保存再退出"),QStringLiteral("确定"),QStringLiteral("取消")))
    {
        e->accept();
    }else{
        e->ignore();
    }
}

///初始化导入脚本对话框的内容
void MainWindow::initImportScriptDialog(){
    //选择文本后响应函数
    connect(isDialog,&ImportScriptDialog::packageSelected,this,[&](const QString packFile){

        //        QtConcurrent::run([&](){
        //导入c++模块
        //        _moduleLibrary->addPackage(QString(packFile).toStdString());

        //导入子系统flow文件
        //        _moduleLibrary->subsystemLibrary().getInvocableList();
        //        emit _moduleLibrary->importCompleted();
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

    //    connect(_moduleLibrary.get(),&ModuleLibrary::fileParserCompleted,this,[&](int count, int index){
    //        qDebug() << "index:" <<index << "   count:" << count;
    //    });


    //文件解析百分比
    //TODO:此处可能要处理
    //    connect(_moduleLibrary.get(),&ModuleLibrary::fileParserCompleted,this,[&](const int count,const int index,const QString filename){
    //        isDialog->setImportProcess(index,count);
    //        isDialog->setListModels(_moduleLibrary.get());
    //    });
}

void MainWindow::openSubsystem(const std::string package,const std::string name){
    auto subsystemWindow = new SubsystemWindow(_moduleLibrary.get(), _moduleLibrary->subsystemLibrary().getSubsystem(package,name), this);
    //当子系统有node创建或删除时，将信号继续传送到外部
    connect(subsystemWindow,&SubsystemWindow::subsystemCreatedOrDeleted,this,[&]{
        emit subsystemWindow->subsystemCreatedOrDeleted();
    });
    subsystemWindow->show();
}





