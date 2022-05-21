#include "testdialog.h"
#include "ui_testdialog.h"

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_QuitOnClose, false);
    initEvent();


   ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->setShowGrid(false);

    for(int i=0;i<30;i++){
        AICCToolButton *tb = new AICCToolButton();
        tb->setText("text");
        tb->setNodeName("name");
        tb->setNodeCaption("caption");

        qDebug() << ui->tableWidget->columnWidth(0);

        tb->setFixedSize(ui->tableWidget->columnWidth(0)-20,
                         ui->tableWidget->rowHeight(0)-20);
//        tb->setGeometry(0,0,30,30);
//        tb->setMaximumSize(120,60);
        QSizePolicy sp = tb->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Preferred);
        tb->setSizePolicy(sp);

        ui->tableWidget->setCellWidget(i,0,tb);
        ui->tableWidget->resizeColumnToContents(0);
        ui->tableWidget->resizeRowToContents(i);
    }
}

void TestDialog::initEvent(){
    qDebug() << "init event";
    connect(ui->tableWidget,&QTableWidget::cellPressed,this,[&](int row, int column){
        //        if(col==LAYERFLDCOL)
        qDebug() << "cell pressed";


    });
}


TestDialog::~TestDialog()
{
    delete ui;
}

