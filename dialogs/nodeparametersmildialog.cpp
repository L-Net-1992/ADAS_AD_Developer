#include "nodeparametersmildialog.h"
#include "ui_nodeparametersmildialog.h"

NodeParametersMILDialog::NodeParametersMILDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeParametersMILDialog)
{
    ui->setupUi(this);
        this->setAttribute(Qt::WA_QuitOnClose, false);
    initToolButton();
}


NodeParametersMILDialog::~NodeParametersMILDialog()
{
    delete ui;
}

void NodeParametersMILDialog::initToolButton(){
    QList<QToolButton *> tb_list;
    tb_list.append(ui->tb_sensor1);
    tb_list.append(ui->tb_sensor2);
    tb_list.append(ui->tb_sensor3);
    tb_list.append(ui->tb_sensor4);

    QList<QLineEdit *> le_list;
    le_list.append(ui->le_sensor1);
    le_list.append(ui->le_sensor2);
    le_list.append(ui->le_sensor3);
    le_list.append(ui->le_sensor4);

    for(int i=0;i<tb_list.count();i++){
        QToolButton * tb = tb_list[i];
        QLineEdit * le = le_list[i];
        connect(tb,&QToolButton::clicked,this,[=](){
            QString file = QFileDialog::getOpenFileName(this,"请选择数据文件",QString(),"CAN数据blf格式 (*.blf);;CAN数据asc格式(*.asc)",Q_NULLPTR,QFileDialog::ReadOnly);
            if(file!=Q_NULLPTR)
                le->setText(file);
        });
    }
}
