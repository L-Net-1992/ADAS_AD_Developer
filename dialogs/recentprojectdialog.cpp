#include "recentprojectdialog.h"
#include "ui_recentprojectdialog.h"

RecentProjectDialog::RecentProjectDialog( ProjectDataModel *pdm ,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecentProjectDialog),
    _project_data_model(pdm)
{
    ui->setupUi(this);
    this->setModal(true);
    initObserver();
    initPlatformConfig();
    initConnect();
}

RecentProjectDialog::~RecentProjectDialog()
{
    delete ui;
}


void RecentProjectDialog::initPlatformConfig(){
    QString platformPath = QApplication::applicationDirPath().append("/platform.xml");
    QFile pfile(platformPath);
    QDomDocument doc;
    //如果没有平台项目文件则创建一个新的
    if(!pfile.exists()){
        pfile.open(QIODevice::ReadWrite | QIODevice::Text);
        QDomProcessingInstruction ins;
        ins = doc.createProcessingInstruction("xml","version=\'1.0\' encoding=\'UTF-8\'");
        doc.appendChild(ins);
        QDomElement root = doc.createElement("platform");
        QDomElement projects = doc.createElement("projects");
        root.appendChild(projects);
        doc.appendChild(root);

        QTextStream stream(&pfile);
        stream.setCodec("UTF-8");

        doc.save(stream,4);         //4表示缩进的空格数
        pfile.close();
    }

    //打开平台项目文件
    if(pfile.open(QIODevice::ReadOnly) && doc.setContent(&pfile)){
        QDomElement root = doc.documentElement();
        QDomNodeList nlist = root.elementsByTagName("projects");
        QDomNode nProjects = nlist.at(0);

        QDomNodeList nProject = nProjects.childNodes();

        for(int i=0;i<nProject.count();i++){
            QString pname = nProject.at(i).toElement().attributeNode("name").value();
            QString ppath = nProject.at(i).toElement().attributeNode("path").value();
            _project_data_model->addRecentProject(pname,ppath);
        }
    }
}

///初始化观察者控件
void RecentProjectDialog::initObserver(){
    connect(_project_data_model,&ProjectDataModel::addRecentProjectEvent,this,[&](const  QString name,const  QString path){
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(QSize(10,70));
        setDoubleClickAction(item);
        ui->lw_project->addItem(item);

        //定义每行条目的模板
        QWidget *w = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout;
        QLabel *l_name = new QLabel(name,w);
        l_name->setObjectName("l_name");
        QFont f_name ("Ubuntu",12,75);
        l_name->setFont(f_name);

        QLabel *l_path = new QLabel(path,w);
        l_path->setObjectName("l_path");

        //判断项目在路径中是否存在,项目文件是否在路径中存在,如果不存在
        if(!existProject(path)){
            l_name->setStyleSheet("color:red");
            l_path->setStyleSheet("color:red");
        }

        layout->addWidget(l_name);
        layout->addWidget(l_path);
        w->setLayout(layout);
        ui->lw_project->setItemWidget(item,w);

    });
}

///设置点击动作
void RecentProjectDialog::setDoubleClickAction(QListWidgetItem *li){
//    connect(ui->lw_project,&QListWidget::itemClicked,this,[&](){});
    connect(ui->lw_project,&QListWidget::itemDoubleClicked,this,[&](QListWidgetItem *item){
        this->openProject(item);

//        qDebug() << "item double clicked";
    });
}

///初始化所有事件
void RecentProjectDialog::initConnect(){
    connect(this,&RecentProjectDialog::rejected,this,[&]{
        emit recentProjectDialogClosed();
    });
    connect(ui->pb_close,&QPushButton::clicked,this,[&]{
        this->reject();
    });
    connect(ui->pb_new_project,&QPushButton::clicked,this,[&]{
        emit newProjectTriggered();
    });
    connect(ui->pb_open_project,&QPushButton::clicked,this,[&]{
        if(ui->lw_project->selectedItems().count()>0){
            openProject(ui->lw_project->selectedItems().at(0));
        }else{
            QMessageBox::information(Q_NULLPTR,"信息","请在列表中先选择一个可用项目",QMessageBox::Ok,QMessageBox::Ok);
        }
    });
}

///打开项目操作
void RecentProjectDialog::openProject(QListWidgetItem *item){
    QWidget *widget = ui->lw_project->itemWidget(item);
    if(widget!=Q_NULLPTR){
        QList<QLabel *> labelList = widget->findChildren<QLabel*>();
        foreach(QLabel *label,labelList){
            if(label->objectName()=="l_name")
                _project_data_model->setCurrentProjectName(label->text());
            else if(label->objectName()=="l_path")
                _project_data_model->setCurrentProjectPath(label->text());
        }
        if(!existProject(_project_data_model->currentProjectPath())){
            QMessageBox::warning(Q_NULLPTR,"警告","项目不存在",QMessageBox::Ok,QMessageBox::Ok);
            return;
        }

        //当数据点击item设置完当前项目的数据后，发送消息通知主窗口当前项目信息
        emit setCurrentProjectDataModelCompleted(_project_data_model);
        this->hide();
    }
}

///判断项目是否存在
bool RecentProjectDialog::existProject(const QString ppath){
    QDir existDir(ppath);
    QFile existFile(ppath+"/.ap/project.xml");
    if(!existDir.exists() || !existFile.exists()) return false;
    return true;
}
