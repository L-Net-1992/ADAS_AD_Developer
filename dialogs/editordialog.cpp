#include "editordialog.h"
#include "ui_editordialog.h"
//#include

EditorDialog::EditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditorDialog)
{
    ui->setupUi(this);
}

void EditorDialog::openTextFile(const QString pathName){
    QFile file(pathName);
    if(file.open(QIODevice::ReadOnly|QIODevice::Text)){
        ui->pte_editor->clear();
        ui->pte_editor->appendPlainText(file.readAll());
    }
    else
        qDebug() <<"The file open failed!";
}

EditorDialog::~EditorDialog()
{
    delete ui;
}
