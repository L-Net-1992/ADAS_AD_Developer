#include "monitordialog.h"
#include "ui_monitordialog.h"
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QChart>
#include "hdf5/hdf5files_handle.h"

using namespace utility;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // clear buffer
    signal_name_list_.clear();
    signals_data_int.clear();
    signals_data_float.clear();

    // btn_monitor_start
    ui->btn_monitor_start->setText("run");
    connect(ui->btn_monitor_start, &QPushButton::clicked, this, [=](){
        QHeaderView *header_view = ui->tb_signal->verticalHeader();
        header_view->setHidden(true);
        ui->tb_signal->horizontalHeader()->setStretchLastSection(true);
    });

    // btn_replay_start
    ui->btn_replay_start->setText("replay");
    connect(ui->btn_replay_start, &QPushButton::clicked, this, [=](){
        if(ui->btn_replay_start->text() == "replay"){
            ui->btn_replay_start->setText("stop");
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/res/pause.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->btn_replay_start->setIcon(icon);
            ui->btn_replay_start->setIconSize(QSize(40, 40));
            ui->btn_replay_start->setToolButtonStyle(Qt::ToolButtonIconOnly);
            ui->btn_replay_start->setAutoRaise(false);

            timer1->start();

        }
        else {
            ui->btn_replay_start->setText("replay");
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/res/replay.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->btn_replay_start->setIcon(icon);
            ui->btn_replay_start->setIconSize(QSize(40, 40));
            ui->btn_replay_start->setToolButtonStyle(Qt::ToolButtonIconOnly);
            ui->btn_replay_start->setAutoRaise(false);

            timer1->stop();
        }
    });

    // btn_replay_open
    connect(ui->btn_replay_open, &QPushButton::clicked, this, [=](){
        Hdf5Handle file1;
        QString str = QFileDialog::getOpenFileName(this,"open file", QApplication::applicationDirPath(),tr("HDF5 (*.h5 *.hdf5)"));

        //
        if(str == "") {
            return;
        }
        hid_t ret = file1.open(str.toStdString());
        if(ret == 1) {
            // new_file.read();
            signal_name_list_.clear();
            signal_name_list_ = file1.get_list("/Signal");

            // update table content
            update_table_content(signal_name_list_.size());

            // save files date to buffer
            for(int i=0; i<signal_name_list_.size();++i) {
                int val = file1.get_class(signal_name_list_[i], "/Signal");
                if(val==0) {
                    std::vector<int> arr_int = file1.get_data<int>(signal_name_list_[i], "/Signal");
                    signals_data_int[signal_name_list_[i]] = arr_int;
                } else {
                    std::vector<float> arr_float = file1.get_data<float>(signal_name_list_[i], "/Signal");
                    signals_data_float[signal_name_list_[i]] = arr_float;
                }
            }

            file1.close();
        }
        else {
            QMessageBox msg = QMessageBox(this);
            msg.warning(this, "warning", "wrong file", QMessageBox::Ok, QMessageBox::Ok);
        }
    });


//    //Record data
//    connect(ui->btn_save,&QPushButton::clicked,this,[&]{
//        QJsonObject hdf5_save;

//        QJsonDocument qjdoc;
//        qjdoc.setObject(hdf5_save);
//        qDebug() << hdf5_save;

//        //TODO::此处保存文件位置以后可以设置到项目目录中
//        QString spath = QFileDialog::getSaveFileName(this,tr("Save File"),QApplication::applicationDirPath(),tr("Calibration Data (*.hdf5)"));
//        if(QFileInfo(spath).suffix()!="json"||QFileInfo(spath).suffix().isEmpty())
//            spath+=".hdf5";
//        QSharedPointer<QFile> save_file(new QFile(spath));
//        save_file->open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text);
//        //save_file->write(qjdoc.toJson());
//        save_file->close();
//    });


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
    headers << QStringLiteral("选择") << QStringLiteral("图例") << QStringLiteral("信号") << QStringLiteral("值") ;
    ui->tb_signal->setHorizontalHeaderLabels(headers);

    //Temp Data
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


        //Name
        QTableWidgetItem *itemName=ui->tb_signal->item(i,2);
        QString item = itemName->text();

        //Line  ▃▃▃▃▃
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

    // timer1
    timer1 = new QTimer(this);
    timer1->setInterval(200);
    connect(timer1, &QTimer::timeout, this, [=](){
        // update signal list
        if(signal_name_list_.size()) {
            for(int i=0; i<signal_name_list_.size();++i) {
                QTableWidgetItem *it = new QTableWidgetItem("");
                QString str = signal_name_list_.at(i).c_str();
                it->setText(str);
                ui->tb_signal->setItem(i,2,it);
            }
        }

        //

    });


    // timer2
    timer2 = new QTimer(this);
    timer2->setInterval(200);
    connect(timer2, &QTimer::timeout, this, [=](){
            qDebug() << "timer2 update";
    });
    //timer2->start();

}

QCheckBox * Dialog::getCheckBox(int row, int column)
{

}
Dialog::~Dialog()
{
    delete ui;
}

void Dialog::update_table_content(int number)
{
    ui->tb_signal->clearContents();

    //Temp Data
    for(int i=0;i<number;i++){
        //Checkbox
        QCheckBox *ckb = new QCheckBox(this);
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(ckb);
        hLayout->setMargin(0);
        hLayout->setAlignment(ckb,Qt::AlignCenter);
        QWidget *wckb = new QWidget(ui->tb_signal);
        wckb->setLayout(hLayout);
        ui->tb_signal->setCellWidget(i,0,wckb);

        // color
        QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
        itemLine->setTextColor(QColor(rand()%256,rand()%256,rand()%256));
        ui->tb_signal->setItem(i,1,itemLine);

        // name
        QTableWidgetItem *it = new QTableWidgetItem("");
        QString str = signal_name_list_.at(i).c_str();
        it->setText(str);
        ui->tb_signal->setItem(i,2,it);
    }
}
