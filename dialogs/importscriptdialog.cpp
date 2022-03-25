#include "importscriptdialog.h"
#include "ui_importscriptdialog.h"

ImportScriptDialog::ImportScriptDialog(QSharedPointer<ProjectDataModel>pdm,QWidget *parent) :
    QDialog(parent),
    _projectDataModel(pdm),
    ui(new Ui::ImportScriptDialog)
{
    ui->setupUi(this);
    this->initButton();
}

ImportScriptDialog::~ImportScriptDialog()
{
    delete ui;
}

///初始化导入按钮操作
void ImportScriptDialog::initButton(){
//    connect(ui->pb_import,&QPushButton::clicked,this,[&](){
//        QStringList files = QFileDialog::getOpenFileNames(this,"请选择功能模块包文件adas-package.json",QString(),"adas-package.json(adas-package.json)",Q_NULLPTR,QFileDialog::ReadOnly);
//        if(!files.isEmpty()){
//            emit filesSelected(files);
//        }
//    });

    connect(ui->pb_import,&QPushButton::clicked,this,[&]{
//         QString file = QFileDialog::getOpenFileName(this,"清选择功能模块包文件adas-package.json",QString(),"adas-package.json(adas-package.json)",Q_NULLPTR,QFileDialog::ReadOnly);
        QString path = QFileDialog::getExistingDirectory(this,"请选择功能模块包",_projectDataModel->projectPath());
         if(!path.isEmpty()){
             emit packageSelected(path);
         }
    });
}

///设置进度条百分比
void ImportScriptDialog::setImportProcess(const int cvalue,const int mvalue){
    ui->progressBar->setRange(0,mvalue);
    ui->progressBar->setValue(cvalue+1);
}

void ImportScriptDialog::setListModels(ModuleLibrary *moduleLibrary){
    ui->list->setModel(moduleLibrary);
}
