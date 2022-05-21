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

    //是否屏蔽
//    ui->captionLabel->setVisible(false);
//    ui->captionLineEdit->setVisible(false);
//    ui->categoryComboBox->setVisible(false);
//    ui->categoryLabel->setVisible(false);

}

NewSubsystemDialog::~NewSubsystemDialog() {
    delete ui;
}

NewSubsystemDialog::SubsystemNameType NewSubsystemDialog::getSubsystemName() {
    return {ui->packageEdit->text().toStdString(), ui->nameEdit->text().toStdString()};
}

NewSubsystemDialog::SubsystemDataModel NewSubsystemDialog::getSubsystemDataModel(){
    SubsystemDataModel dataModel;
    dataModel.insert(make_pair("category",ui->categoryComboBox->currentText().toStdString()));
    dataModel.insert(make_pair("package",ui->packageEdit->text().toStdString()));
    dataModel.insert(make_pair("name",ui->nameEdit->text().toStdString()));
    dataModel.insert(make_pair("caption",ui->captionEdit->text().toStdString()));
    return dataModel;
}


void NewSubsystemDialog::setCategoryComboBox(QStringList categoryDataModel){
//    qDebug() << "set category combobox:" << categoryDataModel.size();
    QStringList::iterator it;
    for(it=categoryDataModel.begin();it!=categoryDataModel.end();it++){
        std::string s = it->toStdString();
        if(s=="") continue;
        ui->categoryComboBox->addItem(QString::fromStdString(it->toStdString()));
    }
}




























