#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"

CalibrationDialog::CalibrationDialog(QWidget *parent,QString pp) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog),
    project_path(pp)
{
    ui->setupUi(this);
    init();
    initButton();
}

CalibrationDialog::~CalibrationDialog()
{
    delete ui;
}

///初始化界面内容
void CalibrationDialog::init(){
    //表格列宽自适应
    ui->tw_params->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    //工具条按钮靠右
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->toolBar->addAction(ui->action_sync_to_module);
    ui->toolBar->addWidget(spacer);
    ui->toolBar->addAction(ui->action_save);
    ui->toolBar->addAction(ui->action_load);
    ui->toolBar->addAction(ui->action_update);

    //设置第三列为只读,但此方法会覆盖掉原来的数据
    for(int i=0;i<ui->tw_params->columnCount();i++){
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setText("3");
        ui->tw_params->setItem(i,2,item);
    }
}

///初始化按钮事件
void CalibrationDialog::initButton(){
    //Sync to Module按钮
    connect(ui->action_sync_to_module,&QAction::triggered,this,[&]{

    });

    //Save按钮
    connect(ui->action_save,&QAction::triggered,this,[&]{
        QJsonObject json_save ;
        QTableWidget * tw = ui->tw_params;
        for(int i=0;i<tw->rowCount();i++){
            QString param_name;
            QString current_value;
            for(int j=0;j<tw->columnCount();j++){
                if(tw->item(i,j)!=Q_NULLPTR){
                    if(j==0) param_name = tw->item(i,j)->text();
                    if(j==2) current_value = tw->item(i,j)->text();
                }
            }
            json_save.insert(param_name,current_value);
        }

        QJsonDocument qjdoc;
        qjdoc.setObject(json_save);
        qDebug() << json_save;

        //TODO::此处保存文件位置以后可以设置到项目目录中
        QString spath = QFileDialog::getSaveFileName(this,tr("Save File"),QApplication::applicationDirPath(),tr("Calibration Data (*.json)"));
        spath+=".json";
        QSharedPointer<QFile> save_file(new QFile(spath));
        save_file->open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text);
        save_file->write(qjdoc.toJson());
        save_file->close();
    });

    //Load按钮
    connect(ui->action_load,&QAction::triggered,this,[&]{

    });
    //Update按钮
}
