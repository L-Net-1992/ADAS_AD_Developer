#include "monitordialog.h"
#include "ui_monitordialog.h"
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QChart>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // btn_replay
    connect(ui->btn_replay, &QPushButton::clicked, this, [=](){
        QString str = QFileDialog::getOpenFileName(this,"open file","./");

        QFile file(str);
        file.open(QIODevice::ReadOnly);

        QByteArray array;
        std::map<std::string,int>mp;
        while(!file.atEnd()) {
            array = file.readLine();
            QString str(array);
            qDebug() << str;
        }

        qDebug() << "path: " << str;
        file.close();
    });

    // table
    ui->tb_signal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    ui->tb_signal->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
//    ui->tb_signal->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
//    ui->tb_signal->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
//    ui->tb_signal->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);
    ui->tb_signal->setRowCount(4);
    ui->tb_signal->setColumnCount(4);

//    ui->tb_signal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList headers;
    headers << QStringLiteral("") << QStringLiteral("") << QStringLiteral("Name") << QStringLiteral("Value") ;
    ui->tb_signal->setHorizontalHeaderLabels(headers);


    //
    QCheckBox *ckb = new QCheckBox(this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(ckb);
    hLayout->setMargin(0);
    hLayout->setAlignment(ckb,Qt::AlignCenter);
    QWidget *wckb = new QWidget(ui->tb_signal);
    wckb->setLayout(hLayout);
    ui->tb_signal->setCellWidget(0,0,wckb);

    //
    QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
    itemLine->setTextColor(QColor(rand()%256,rand()%256,rand()%256));
    ui->tb_signal->setItem(0,1,itemLine);

    // chart
    QChart *chart = new QChart();
    ui->chart->setChart(chart);
    QSplineSeries *series = new QSplineSeries;

    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0,20);
    axisX->setLabelFormat("%u");
    axisX->setGridLineVisible(true);
    axisX->setTickCount(10);
    axisX->setMinorTickCount(1);
    axisX->setTitleText("X");

    QValueAxis *axisY = new QValueAxis();
    axisY = new QValueAxis;
    axisY->setRange(0,10);
    axisY->setLabelFormat("%u");
    axisY->setGridLineVisible(true);
    axisY->setTickCount(10);
    axisY->setMinorTickCount(1);
    axisY->setTitleText("Y");

    chart->addAxis(axisX,Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);

}

QCheckBox * Dialog::getCheckBox(int row, int column)
{

}
Dialog::~Dialog()
{
    delete ui;
}

