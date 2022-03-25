#include "exportmoduledialog.h"
#include "ui_exportmoduledialog.h"

ExportModuleDialog::ExportModuleDialog(QSharedPointer<ProjectDataModel>pdm,QWidget *parent) :
    QDialog(parent),
    pDataModel(pdm),
    ui(new Ui::ExportModuleDialog)
{
    ui->setupUi(this);
    initConnect();

}

ExportModuleDialog::~ExportModuleDialog()
{
    delete ui;
}

///显示事件
void ExportModuleDialog::showEvent(QShowEvent *event) {
    getSubsystemData();
}

///初始化打开的项目下的子系统数据
void ExportModuleDialog::getSubsystemData(){
    QVector<QString> vectorSubsystemModuleName;
    ui->cb_module_name->clear();

    QString subsystemPath = pDataModel->projectSubSystemPath();
    //遍历目录下的所有子系统目录
    QDir dir(subsystemPath);
    if(!dir.exists()) return ;
    dir.setFilter(QDir::Dirs);
    QFileInfoList dirList = dir.entryInfoList();
    for(int i=0;i<dirList.count();i++){
        QFileInfo fi = dirList.at(i);

        //如果路径为 . 或者 .. 则继续循环
        if(fi.baseName()=="") continue;
        QString packageName = dirList.at(i).baseName();
        //遍历每个文件夹下的所有文件
        QString modulePath = fi.filePath();

        //        qDebug() << fi.baseName() << " " << fi.suffix();
        QDir fileDir(modulePath);
        QStringList fileFilters ;
        fileFilters << "*.flow";
        QStringList filesList = fileDir.entryList(fileFilters,QDir::Files|QDir::Readable,QDir::Name);
        for(int j=0;j<filesList.count();j++){
            QString className = filesList.at(j).split(".").at(0);
//            vectorSubsystemModuleName.append(packageName+"::"+className);
            //模块完整路径以万能数据方式带入
            QVariant userData;
            userData.setValue<QString>(subsystemPath);
            ui->cb_module_name->addItem(QString(packageName+"::"+className),userData);
        }
    }
}

///初始化连接
void ExportModuleDialog::initConnect(){
    connect(ui->pb_export,&QPushButton::clicked,this,[&](){
        QFileDialog fileDialog;
        fileDialog.setFileMode(QFileDialog::Directory);
        QString spath = fileDialog.getExistingDirectory(this,tr("请选择导出位置"),pDataModel->projectPath());

        //获得要导出插件的包名
        QString moduleName = ui->cb_module_name->currentText();
        QString packageName = moduleName.split("::").at(0);
        //判断目标目录下是否有指定包，如果没有指定包创建目录
        QDir packagePath(spath+"/"+packageName);
        if(!packagePath.exists()) packagePath.mkdir(packagePath.absolutePath());
        //拷贝文件到目标文件夹中
        QString srcFile = pDataModel->projectSubSystemPath()+QString("/").append(moduleName.split("::").at(0)).append("/").append(moduleName.split("::").at(1)).append(".flow");
        QString dstFile = spath.append("/").append(moduleName.split("::").at(0)).append("/").append(moduleName.split("::").at(1)).append(".flow");
//        qDebug() << " source file:" << srcFile << "  dist file:" << dstFile;
        if(copyFile(srcFile,dstFile,true))
            this->close();
    });
}















































