#include "exportmoduledialog.h"
#include "ui_exportmoduledialog.h"

ExportModuleDialog::ExportModuleDialog(ProjectDataModel *pdm,QWidget *parent) :
    QDialog(parent),
    pDataModel(pdm),
    ui(new Ui::ExportModuleDialog)
{
    ui->setupUi(this);

}

ExportModuleDialog::~ExportModuleDialog()
{
    delete ui;
}


///初始化子系统数据
void ExportModuleDialog::initSubsystemData(){
    QString path = pDataModel->currentProjectSubSystemPath();
    //遍历目录下的所有子系统目录
    QDir dir(path);
    if(!dir.exists()) return ;
    dir.setFilter(QDir::Dirs);
    QFileInfoList list = dir.entryInfoList();
    for(int i=0;i<list.count();i++){
        QFileInfo fi = list.at(i);
        qDebug()<< "init subsystem data:" << fi.dir().path();
        //遍历每个文件夹下的所有文件
        QString subsystemPath = path+fi.dir().path();
        QDir fileDir(subsystemPath);
        QStringList fileFilters ;
//        fileFilters << ""


    }
}

