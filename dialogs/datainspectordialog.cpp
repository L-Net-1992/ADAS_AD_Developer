#include "datainspectordialog.h"
#include "ui_datainspectordialog.h"



DataInspectorDialog::DataInspectorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataInspectorDialog)
{
    ui->setupUi(this);
    initTableWidget();
    initCharts();
}

DataInspectorDialog::~DataInspectorDialog()
{
    delete ui;
}

///
void DataInspectorDialog::initTableWidget(){
    ui->tw_output->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tw_output->setColumnWidth(0,23);
    ui->tw_output->setColumnWidth(1,130);
    ui->tw_output->setColumnWidth(2,85);

    //Temp Data
    ui->tw_output->setRowCount(3);
    for(int i=0;i<ui->tw_output->rowCount();i++){
        //Check Box
        QCheckBox *ckb = new QCheckBox(this);
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(ckb);
        hLayout->setMargin(0);
        hLayout->setAlignment(ckb,Qt::AlignCenter);
        QWidget *wckb = new QWidget(ui->tw_output);
        wckb->setLayout(hLayout);
        ui->tw_output->setCellWidget(i,0,wckb);

        //Name
        QTableWidgetItem *itemName = new QTableWidgetItem("Output"+i);
        ui->tw_output->setItem(i,1,itemName);

        //Line
        QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
        itemLine->setTextColor(QColor(rand()%256,rand()%256,rand()%256));
        ui->tw_output->setItem(i,2,itemLine);
        //        QGraphicsLineItem *itemLine = new QGraphicsLineItem();
        //        itemLine->setLine(0,0,0,30);
        //        QHBoxLayout *lLayout = new QHBoxLayout(this);
        //        lLayout->addWidget(itemLine);
        //        hLayout->setMargin(0);
        //        hLayout->setAlignment(itemLine,Qt::AlignCenter);

        //        QWidget *wline = new QWidget(ui->tw_output);
        //        QGraphicsWidget *gw = new QGraphicsWidget();

        //        ui->tw_output->setItem(i,2,itemLine);
    }
}


void DataInspectorDialog::initCharts(){

    QChart *chart = new QChart();
    ui->chartView->setChart(chart);
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
    chart->setAxisX(axisX,series);
    chart->setAxisY(axisY,series);

//    QHBoxLayout *layout = new QHBoxLayout(this);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}
