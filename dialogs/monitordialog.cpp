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
        QString str = QFileDialog::getOpenFileName(this,"open file","./","*.hdf5");

        QFile file(str);
        file.open(QIODevice::ReadOnly);

        QByteArray array;
        std::map<std::string,int>mp;
        while(!file.atEnd()) {
            array = file.readLine();
            QString str(array);
            //qDebug() << str;
        }

        //qDebug() << "path: " << str;
        file.close();
    });

    //Record data
    connect(ui->btn_save,&QPushButton::clicked,this,[&]{
        QJsonObject hdf5_save;

        QJsonDocument qjdoc;
        qjdoc.setObject(hdf5_save);
        qDebug() << hdf5_save;

        //TODO::此处保存文件位置以后可以设置到项目目录中
        QString spath = QFileDialog::getSaveFileName(this,tr("Save File"),QApplication::applicationDirPath(),tr("Calibration Data (*.hdf5)"));
        if(QFileInfo(spath).suffix()!="json"||QFileInfo(spath).suffix().isEmpty())
            spath+=".hdf5";
        QSharedPointer<QFile> save_file(new QFile(spath));
        save_file->open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text);
        //save_file->write(qjdoc.toJson());
        save_file->close();
    });


    // table
    //ui->tb_signal->setRowCount(4);
    ui->tb_signal->setColumnCount(4);
    //ui->tb_signal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tb_signal->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    //ui->tb_signal->setColumnWidth(0,1);
    ui->tb_signal->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    //ui->tb_signal->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
    ui->tb_signal->setColumnWidth(2,150);
    ui->tb_signal->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);

//    ui->tb_signal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList headers;
    headers << QStringLiteral("Check") << QStringLiteral("Legend") << QStringLiteral("Name") << QStringLiteral("Value") ;
    ui->tb_signal->setHorizontalHeaderLabels(headers);

    //Temp Data
    ui->tb_signal->setRowCount(3);
    for(int i=0;i<ui->tb_signal->rowCount();i++){
    //Checkbox
        QCheckBox *ckb = new QCheckBox(this);
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(ckb);
        hLayout->setMargin(0);
        hLayout->setAlignment(ckb,Qt::AlignCenter);
        QWidget *wckb = new QWidget(ui->tb_signal);
        wckb->setLayout(hLayout);
        ui->tb_signal->setCellWidget(i,0,wckb);
//        QTableWidgetItem *ckb=new QTableWidgetItem();
//        ckb->setCheckState(Qt::Unchecked);
//        ui->tb_signal->setItem(i,0,ckb);


    //Name
        QTableWidgetItem *itemName = new QTableWidgetItem("Output"+QString::number(i*99));
        ui->tb_signal->setItem(i,2,itemName);
        QString item = itemName->text();
        //qDebug()<<item;
    //Line
    //        ▃▃▃▃▃
        QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
        //itemLine->setTextColor(QColor(rand()%256,rand()%256,rand()%256));
        QByteArray byte = item.toUtf8();
        int color=0;
        for(int j=0;j<item.size();j++){
            color=color+int(byte.at(j));
        }
        itemLine->setTextColor(QColor((color*2)%256,(color*3)%256,(color*5)%256));
        ui->tb_signal->setItem(i,1,itemLine);
    }
    //
    //QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
    //itemLine->setTextColor(QColor(rand()%256,rand()%256,rand()%256));
   // ui->tb_signal->setItem(0,1,itemLine);

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
    //axisY = new QValueAxis;
    axisY->setRange(0,10);
    axisY->setLabelFormat("%u");
    axisY->setGridLineVisible(true);
    axisY->setTickCount(10);
    axisY->setMinorTickCount(1);
    axisY->setTitleText("Y");

    chart->addAxis(axisX,Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);

    series->setPen(QPen(Qt::red,1,Qt::SolidLine));
    series->append(0,6);
    series->append(2,4);
    series->append(3,8);
    series->append(6,4);
    series->append(8,6);
    series->append(10,5);

    *series << QPointF(11,1) << QPointF(13,3) << QPointF(17,6) << QPointF(18,3) << QPointF(20,2);

    chart->setTitle("chart");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->addSeries(series);
    ui->chart->setRenderHint(QPainter::Antialiasing);

}

QCheckBox * Dialog::getCheckBox(int row, int column)
{

}
Dialog::~Dialog()
{
    delete ui;
}

