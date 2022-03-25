#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"

CalibrationDialog::CalibrationDialog(const QString ip,ProjectDataModel *pdm,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog),
    _projectDataModel(pdm),
    inspector(ip)
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

    this->setAttribute(Qt::WA_DeleteOnClose);

    //工具条按钮靠右
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//    ui->toolBar->addAction(ui->action_sync_to_module);
//    ui->toolBar->addWidget(spacer);
    ui->toolBar->addAction(ui->action_save);
    ui->toolBar->addAction(ui->action_load);
    ui->toolBar->addAction(ui->action_update);

    // clear table content
    while(ui->tw_params->rowCount()) {
        ui->tw_params->removeRow(0);
    }

    //获得所有可标定参数的列表，map中key为名字，value为当前值
    auto param_value = inspector.getParamValue();
    qDebug() << "params: " << param_value;
    for(auto it=param_value.begin();it!=param_value.end();++it) {
        auto rowcount = ui->tw_params->rowCount();
        ui->tw_params->setRowCount(rowcount + 1);

        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setText(it.key());
        ui->tw_params->setItem(rowcount,0,item);

        QTableWidgetItem *item2 = new QTableWidgetItem("");
        item2->setText(QString("%1").arg(it.value()));
        ui->tw_params->setItem(rowcount,1,item2);
    }

    //设置第三列为只读,但此方法会覆盖掉原来的数据
//    for(int i=0;i<ui->tw_params->rowCount();i++){
//        QTableWidgetItem *item = new QTableWidgetItem();
//        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
//        //item->setText("3");
//        ui->tw_params->setItem(i,1,item);
//    }
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
                    if(j==1) current_value = tw->item(i,j)->text();
                }
            }
            json_save.insert(param_name,current_value);
        }

        QJsonDocument qjdoc;
        qjdoc.setObject(json_save);
        qDebug() << json_save;

        QString spath = QFileDialog::getSaveFileName(this,tr("Save File"),_projectDataModel->currentProjectPath(),tr("标定数据json格式 (*.json)"));
        if(QFileInfo(spath).suffix()!="json"||QFileInfo(spath).suffix().isEmpty())
            spath+=".json";
        QSharedPointer<QFile> save_file(new QFile(spath));
        save_file->open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text);
        save_file->write(qjdoc.toJson());
        save_file->close();
    });

    //Load按钮
    connect(ui->action_load,&QAction::triggered,this,[&]{
        QString param_name;
        QTableWidget * tw = ui->tw_params;

        //save the path of the selected file and open it.
        QString lpath = QFileDialog::getOpenFileName(this,tr("Load File"),QApplication::applicationDirPath(),tr("标定数据json格式 (*.json)"));
        QFile load_file(lpath);
        load_file.open(QIODevice::ReadOnly | QIODevice::Text);

        QByteArray value=load_file.readAll();

        QJsonParseError err;
        QJsonDocument json_doc=QJsonDocument::fromJson(value,&err);
        if(json_doc.isNull()){
            qDebug()<<err.errorString();
        }

        //fill the new value from the seclectd file to calibration table followed the key value
        QJsonObject json_load=json_doc.object();
            for(int i=0;i<tw->rowCount();i++){
                param_name = tw->item(i,0)->text();
                if(!json_load.value(param_name).isNull()){
                    tw->setItem(i,2,new QTableWidgetItem(json_load.value(param_name).toString()));
                }
            }
    });

    //Update按钮
    connect(ui->action_update,&QAction::triggered,this,[=](){
        //标定参数，通过map可以一次设置多个变量

        for(int i=0;i<ui->tw_params->rowCount();i++) {
            QString name = ui->tw_params->item(i,0)->text();
            auto setval =ui->tw_params->item(i,2);
            if(setval != 0) {
                QMap<QString, float> set_param;
                set_param[name] = setval->text().toFloat();
                inspector.setParamValue(set_param);

                auto param_value = inspector.getParamValue();
                QTableWidgetItem *item2 = new QTableWidgetItem("");
                item2->setText(QString("%1").arg(param_value.value(name)));
                ui->tw_params->setItem(i,1,item2);
            }
        }

    });
}

void CalibrationDialog::setProjectDataModel(ProjectDataModel *newProjectDataModel)
{
    _projectDataModel = newProjectDataModel;
}
