#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"
#include <QToolButton>
#include <QLineEdit>
#include <QLabel>

CalibrationDialog::CalibrationDialog(const QString ip,QSharedPointer<ProjectDataModel> pdm,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog),
    _projectDataModel(pdm),
    _ip(ip)
{
    ui->setupUi(this);
    init();
    initButton();
}

CalibrationDialog::~CalibrationDialog()
{
    delete ui;

    if(inspector_) {
        delete inspector_;
        inspector_ = nullptr;
    }
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

#if 0 // 在连接按钮中获取
    //获得所有可标定参数的列表，map中key为名字，value为当前值
    try{
        Inspector inspector(_ip);
        auto param_value = inspector.getParamValue();
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

    }catch(std::exception &e){
        qCritical() << "calibrationdialog exception:" << e.what();
    }
#endif


    //原来已经注释 设置第三列为只读,但此方法会覆盖掉原来的数据
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

        QString spath = QFileDialog::getSaveFileName(this,tr("Save File"),_projectDataModel->projectPath(),tr("标定数据json格式 (*.json)"));
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
        QString lpath = QFileDialog::getOpenFileName(this,tr("Load File"),_projectDataModel->projectPath(),tr("标定数据json格式 (*.json)"));
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
        if(inspector_ == nullptr) {
            qDebug() << "No Device Connect";
            return;
        }
        //标定参数，通过map可以一次设置多个变量
        for(int i=0;i<ui->tw_params->rowCount();i++) {
            QString name = ui->tw_params->item(i,0)->text();
            auto setval =ui->tw_params->item(i,2);
            if((setval == nullptr) || (setval->text().isEmpty())) {
                continue;
            }
            else {
                QMap<QString, float> set_param;
                set_param[name] = setval->text().toFloat();
                try{
                    inspector_->setParamValue(set_param);
                    auto param_value = inspector_->getParamValue();
                    QTableWidgetItem *item2 = new QTableWidgetItem("");
                    item2->setText(QString("%1").arg(param_value.value(name)));
                    ui->tw_params->setItem(i,1,item2);
                }catch(std::exception &e){
                    qCritical() << "calibrationdialog exception:" << e.what();
                }
            }
        }

    });

    //
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->toolBar->addWidget(spacer);

    //
    QLabel *label = new QLabel(this);
    label->setObjectName(QString::fromUtf8("label"));
    label->setText(tr("设备地址:"));
    ui->toolBar->addWidget(label);

    //
    QLineEdit *line_edit = new QLineEdit(this);
    line_edit->setObjectName(QString::fromUtf8("lineEdit"));
    line_edit->setText("127.0.0.1");
    line_edit->setMaximumSize(QSize(200, 16777215));
    line_edit->setStyleSheet(QString::fromUtf8("color: rgb(114, 159, 207);"));
    ui->toolBar->addWidget(line_edit);

    //
    QToolButton *btn_device_connect = new QToolButton(this);
    btn_device_connect->setObjectName(QString::fromUtf8("btn_device_connect"));
    btn_device_connect->setText(tr("连接"));
    QIcon icon4;
    icon4.addFile(QString::fromUtf8(":/res/device.png"), QSize(), QIcon::Normal, QIcon::Off);
    btn_device_connect->setIcon(icon4);
    btn_device_connect->setIconSize(QSize(24, 24));
    btn_device_connect->setCheckable(false);
    btn_device_connect->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolBar->addWidget(btn_device_connect);
    connect(btn_device_connect, &QToolButton::clicked, this, [=](){
        if(btn_device_connect->text() == "连接")
        {
            QString deviceip = line_edit->text();
            try {
                inspector_ = new Inspector(deviceip);
            }  catch (...) {
                qDebug() << "Calibration Inspector Connect Wrong";
                return;
            }

            // 清除列表数据
            while(ui->tw_params->rowCount()) {
                ui->tw_params->removeRow(0);
            }
            //获得所有可标定参数的列表，map中key为名字，value为当前值
            try{
                auto param_value = inspector_->getParamValue();
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

                btn_device_connect->setText(tr("断开"));
                line_edit->setEnabled(false);
            }catch(std::exception &e){
                qCritical() << "calibrationdialog exception:" << e.what();
            }

        } else {
            delete inspector_;
            inspector_ = nullptr;

            btn_device_connect->setText(tr("连接"));
            line_edit->setEnabled(true);
        }
    });
}

void CalibrationDialog::setProjectDataModel(QSharedPointer<ProjectDataModel> newProjectDataModel)
{
    _projectDataModel = newProjectDataModel;
}
