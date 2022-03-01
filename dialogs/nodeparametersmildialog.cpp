#include "nodeparametersmildialog.h"
#include "ui_nodeparametersmildialog.h"

NodeParametersMILDialog::NodeParametersMILDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeParametersMILDialog)
{
    ui->setupUi(this);
}

NodeParametersMILDialog::~NodeParametersMILDialog()
{
    delete ui;
}

void NodeParametersMILDialog::initToolButton(){

}
