//
// Created by liudian on 2022/3/13.
//

// You may need to build the project (run Qt uic code generator) to get "ui_var_name_dialog.h" resolved

#include "var_name_dialog.h"
#include "ui_var_name_dialog.h"
#include <QRegExpValidator>


VarNameDialog::VarNameDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::VarNameDialog) {
    ui->setupUi(this);
    QRegExp tokenRegex(R"([a-zA-Z][\w_]*)");
    ui->varNameEdit->setValidator(new QRegExpValidator(tokenRegex, ui->varNameEdit));
}

VarNameDialog::~VarNameDialog() {
    delete ui;
}

QString VarNameDialog::varName() const {
    return ui->varNameEdit->text();
}

void VarNameDialog::setVarName(const QString &var_name) {
    ui->varNameEdit->setText(var_name);

}


