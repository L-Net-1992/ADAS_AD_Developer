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
        QDialog(parent), ui(new Ui::NewSubsystemDialog){
    ui->setupUi(this);
    QRegExp tokenRegex(R"([a-zA-Z][\w_]*)");
    ui->packageEdit->setValidator(new QRegExpValidator(tokenRegex, ui->packageEdit));
    ui->nameEdit->setValidator(new QRegExpValidator(tokenRegex, ui->nameEdit));
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
    dataModel.insert(make_pair("parentid",ui->categoryComboBox->currentData().toString().toStdString()));
    dataModel.insert(make_pair("package",ui->packageEdit->text().toStdString()));
    dataModel.insert(make_pair("name",ui->nameEdit->text().toStdString()));
    dataModel.insert(make_pair("caption",ui->captionEdit->text().toStdString()));
    return dataModel;
}


void NewSubsystemDialog::setCategoryComboBox(std::vector<std::pair<int,std::string>> categoryDataModel){
//    qDebug() << "set category combobox:" << categoryDataModel.size();
    std::vector<std::pair<int,std::string>>::iterator it;
    for(it=categoryDataModel.begin();it!=categoryDataModel.end();it++){
        std::string s = it->second;
        if(s=="") continue;
//        ui->categoryComboBox->
        ui->categoryComboBox->addItem(QString::fromStdString(it->second),it->first);
    }
}

void NewSubsystemDialog::setPackageNameCaption(const QString package,const QString name,const QString caption){
    ui->packageEdit->setText(package);
    ui->packageEdit->setReadOnly(true);
    ui->nameEdit->setText(name);
    ui->nameEdit->setReadOnly(true);
    ui->captionEdit->setText(caption);
}

void NewSubsystemDialog::selectCategoryComboBox(const int id){
    QComboBox *ccb = ui->categoryComboBox;
    for(int idx = 0;idx<ccb->count();idx++){
//        ccb->itemData(idx);
        int pid = ccb->itemData(idx).toInt();
        if(pid == id){
            ccb->setCurrentIndex(idx);
            return ;
        }
    }

}


























