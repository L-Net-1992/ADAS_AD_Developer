#include "exportmoduledialog.h"
#include "ui_exportmoduledialog.h"

ExportModuleDialog::ExportModuleDialog(ProjectDataModel *pdm,QWidget *parent) :
    QDialog(parent),
    pDataModel(pdm),
    ui(new Ui::ExportModuleDialog)
{
    ui->setupUi(this);

//    connect(this,&Q)

    initSubsystemData();
}

ExportModuleDialog::~ExportModuleDialog()
{
    delete ui;
}


///初始化子系统数据
void ExportModuleDialog::initSubsystemData(){
    QVector<QString> subsystemModuleName;

    QString path = pDataModel->currentProjectSubSystemPath();
    qDebug() << "  current project SubSystem path:" << path;
    //遍历目录下的所有子系统目录
    QDir dir(path);
    if(!dir.exists()) return ;
    dir.setFilter(QDir::Dirs);
    QFileInfoList dirList = dir.entryInfoList();
    for(int i=0;i<dirList.count();i++){
        QString packageName = dirList.at(i).baseName();
        qDebug() << " package name:" << packageName;


        QFileInfo fi = dirList.at(i);
        qDebug()<< "init subsystem data:" << fi.dir().path();
        //遍历每个文件夹下的所有文件
        QString subsystemPath = path+fi.dir().path();
        QDir fileDir(subsystemPath);
        QStringList fileFilters ;
        fileFilters << "*.flow";
        QStringList filesList = fileDir.entryList(fileFilters,QDir::Files|QDir::Readable,QDir::Name);
        for(int i=0;i<filesList.count();i++){

        }
    }
}

