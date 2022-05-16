//
// Created by liudian on 2022/3/8.
//

// You may need to build the project (run Qt uic code generator) to get "ui_new_subsystem_dialog.h" resolved

#include "new_subsystem_dialog.h"
#include "ui_new_subsystem_dialog.h"
#include <memory>
#include <QRegExpValidator>
#include <iostream>

NewSubsystemDialog::NewSubsystemDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::NewSubsystemDialog) {
    ui->setupUi(this);
    QRegExp tokenRegex(R"([a-zA-Z][\w_]*)");
    ui->packageEdit->setValidator(new QRegExpValidator(tokenRegex, ui->packageEdit));
    ui->nameEdit->setValidator(new QRegExpValidator(tokenRegex, ui->nameEdit));

    //暂时屏蔽
    ui->captionLabel->setVisible(false);
    ui->captionLineEdit->setVisible(false);
    ui->categoryComboBox->setVisible(false);
    ui->categoryLabel->setVisible(false);

//    connect(_categoryDataModel.get(),&CategoryDataModel::dataLoadCompleted,this,[](){
//        auto f = [](QJson){

//        };
//    });
}

NewSubsystemDialog::~NewSubsystemDialog() {
    delete ui;
}

NewSubsystemDialog::SubsystemNameType NewSubsystemDialog::getSubsystemName() {
    return {ui->packageEdit->text().toStdString(), ui->nameEdit->text().toStdString()};

}

//NewSubsystemDialog::setCategoryComboBox(QMap<QString,QSet<QString>> map){

//}
