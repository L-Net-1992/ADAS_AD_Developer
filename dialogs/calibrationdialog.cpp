#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"

CalibrationDialog::CalibrationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog)
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
    ui->toolBar->addWidget(spacer);
    ui->toolBar->addAction(ui->action_apply_default);
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
    //Apply Default按钮
    connect(ui->action_apply_default,&QAction::triggered,this,[&]{

    });

    //Save按钮
    connect(ui->action_save,&QAction::triggered,this,[&]{
        for(int i=0;i<ui->tw_params->rowCount();i++){
            for(int j=0;j<ui->tw_params->columnCount();j++){
                if(ui->tw_params->item(i,j)!=Q_NULLPTR){

                }
            }
        }
    });

    //Load按钮
    connect(ui->action_load,&QAction::triggered,this,[&]{

    });
    //Update按钮
}
