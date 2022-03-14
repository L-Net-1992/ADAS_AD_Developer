#include "monitordialog.h"
#include "ui_monitordialog.h"
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QChart>
#include <QMessageBox>
#include <QTableWidgetItem>
#include "hdf5/hdf5files_handle.h"

using namespace utility;

#define AXIS_X_SIZE_DEFAULT  (50)

MonitorDialog::MonitorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // clear buffer
    signal_name_list_.clear();
    signals_dataset_.clear();
    color_group_.clear();
    series_group_.clear();
    chartview_list_.clear();

    init_button();
    init_table();
    init_chart();

    // timer1
    timer1 = new QTimer(this);
    timer1->setInterval(20);
    connect(timer1, &QTimer::timeout, this, [=](){
        if(x_index_ >= signal_num_) {
            replay_running_ = 0;
        }

        int rowcount = ui->tb_signal->rowCount();
        for(int i=0;i<rowcount;i++) {
            auto checkbox = (QCheckBox*)ui->tb_signal->cellWidget(i, 0)->children().at(1);
            if(checkbox->checkState()) {
                series_group_.at(i)->setVisible(1);
            } else {
                series_group_.at(i)->setVisible(0);
            }

            // update series data
            if(replay_running_){
                auto name = ui->tb_signal->item(i,2)->text();
                auto it = signals_dataset_.find(name.toStdString());
                if(it != signals_dataset_.end()) {
                    series_group_.at(i)->append(x_index_,it->second.at(x_index_));
                }
                if(it->second.at(x_index_) > y_range_.at(i).at(1)) y_range_.at(i).at(1) = it->second.at(x_index_);
                if(it->second.at(x_index_) < y_range_.at(i).at(0)) y_range_.at(i).at(0) = it->second.at(x_index_);
            }
        }

        // update data index
        if(replay_running_) {
            x_index_ ++;
            if(x_index_ > AXIS_X_SIZE_DEFAULT) {
               axisX_->setRange(x_index_-AXIS_X_SIZE_DEFAULT, x_index_);
            }
            // TODO: select the series 0 as y-axis range
            axisY_->setRange(y_range_.at(0).at(0)-2, y_range_.at(0).at(1)+2);
        }
    });


    // timer2
    timer2 = new QTimer(this);
    timer2->setInterval(200);
    connect(timer2, &QTimer::timeout, this, [=](){
            qDebug() << "timer2 update";
    });
    //timer2->start();

    connect(ui->chart, SIGNAL(mouseMovePoint(QPoint)), this, SLOT(on_mouseMovePoint(QPoint)));
//    connect()
}

QCheckBox * MonitorDialog::getCheckBox(int row, int column)
{

}
MonitorDialog::~MonitorDialog()
{
    delete ui;
}

void MonitorDialog::init_table()
{
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
}

void MonitorDialog::init_chart()
{
//ui->chart->close();
    QChart *chart = new QChart();
//    chart->setTheme(QChart::ChartThemeBrownSand);
    chart->layout()->setContentsMargins(2,2,2,2);
    chart->setMargins(QMargins(0,0,0,0));
    axisX_ = new QValueAxis();
    axisY_ = new QValueAxis();

    QSplineSeries *series = new QSplineSeries(this);
    series->setName("sample");
    series->setPen(QPen(Qt::red,1,Qt::SolidLine));
    series->append(0,6);
    series->append(2,4);
    series->append(3,8);
    series->append(6,4);
    series->append(8,6);
    series->append(10,5);
    *series << QPointF(11,1) << QPointF(13,3) << QPointF(17,6) << QPointF(18,3) << QPointF(20,2);

    chart->addSeries(series);
//    chart->setTitle("信号波形");

//    chart->createDefaultAxes();
    chart->addAxis(axisX_, Qt::AlignBottom);
    chart->addAxis(axisY_, Qt::AlignLeft);
    axisX_->setRange(0, AXIS_X_SIZE_DEFAULT);
//    axisX_->setTickCount(11);
//    axisX_->setMinorTickCount(9);
//    axisX_->setTitleText("Time(s)");
    axisX_->setGridLineVisible(1);
    axisY_->setRange(-10,10);
//    axisY_->setTickCount(6);
    axisY_->setTitleText("Value");
    axisY_->setGridLineVisible(1);
    series->attachAxis(axisX_);
    series->attachAxis(axisY_);

    ui->chart->setChart(chart);
    chart->setAnimationOptions(QChart::NoAnimation);
    ui->chart->setRenderHint(QPainter::Antialiasing);

    chartview_list_.emplace_back(std::move(ui->chart));
    qDebug() << "chartview: " << chartview_list_.size();
    connect(ui->chart, &AiccChartView::sendObjectAddr, this, [=](size_t addr){
        for(auto it=chartview_list_.begin();it!=chartview_list_.end();it++) {
            if( (*it) == (AiccChartView*)addr) {
                qDebug() << "chartview erase: " << *it;
                it = chartview_list_.erase(it);
                break;
            }
        }
    });
    connect(ui->chart, &AiccChartView::currentSelect, this, [=](size_t addr){
        current_chartview_ = (AiccChartView*)addr;
    });
}

void MonitorDialog::init_button()
{
    // btn_monitor_start
    ui->btn_monitor_start->setText("run");
    connect(ui->btn_monitor_start, &QPushButton::clicked, this, [=](){
        QHeaderView *header_view = ui->tb_signal->verticalHeader();
        header_view->setHidden(true);
        ui->tb_signal->horizontalHeader()->setStretchLastSection(true);
    });

    // btn_monitor_stop
    connect(ui->btn_monitor_stop, &QPushButton::clicked, this, [=](){

    });

    // btn_monitor_record
    connect(ui->btn_monitor_record, &QPushButton::clicked, this, [=](){
        QString dir_str = "log";
        QDir dir;
        if(!dir.exists(dir_str)) {
            dir.mkpath(dir_str);
        }

        QDateTime datetime;
        QString timestr = datetime.currentDateTime().toString("yyyy_MM_dd_HH_mm_ss");
        std::string file_name = "log_" + timestr.toStdString() + ".h5";
        std::string path = dir_str.toStdString() + "/" + file_name;
        Hdf5Handle new_file;
        new_file.creat(path);
        if(new_file.get_file_id()) {
            new_file.close();
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

    // btn_replay_start
    ui->btn_replay_start->setText("replay");
    connect(ui->btn_replay_start, &QPushButton::clicked, this, [=](){
        if(ui->btn_replay_start->text() == "replay"){
            ui->btn_replay_start->setText("stop");
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/res/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->btn_replay_start->setIcon(icon);
            ui->btn_replay_start->setIconSize(QSize(40, 40));
            ui->btn_replay_start->setToolButtonStyle(Qt::ToolButtonIconOnly);
            ui->btn_replay_start->setAutoRaise(false);
            ui->btn_replay_start->setToolTip("停止");
            if(!replay_running_) {
                //TODO
                replay_running_ = 1;
            }
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
            ui->btn_replay_start->setToolTip("开始");

            if(x_index_ >= signal_num_) {
                int cnt = series_group_.size();
                for(int i=0;i<cnt;i++) {
                    series_group_[i]->clear();
                }
                x_index_ = 0;
                axisX_->setRange(0, AXIS_X_SIZE_DEFAULT);
                timer1->stop();
                return;
            }
            timer1->stop();

        }
    });

    // btn_replay_open
    connect(ui->btn_replay_open, &QPushButton::clicked, this, [=](){
        if(ui->btn_replay_start->text() == "stop") {
            QMessageBox::warning(this,"提示", "请先关闭回放按键", QMessageBox::Close, QMessageBox::Close);
            return;
        }

        Hdf5Handle file1;
        QString str = QFileDialog::getOpenFileName(this,"open file", QApplication::applicationDirPath(),tr("HDF5 (*.h5 *.hdf5)"));
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
            for(size_t i=0; i<signal_name_list_.size();++i) {
                std::vector<float> arr_float = file1.get_data<float>(signal_name_list_[i], "/Signal");
                signals_dataset_[signal_name_list_[i]] = arr_float;
            }
            signal_num_ = signals_dataset_.begin()->second.size();
            qDebug() << "data size: " << signal_num_;
            file1.close();

            // creat series for signals
            int rowcount = ui->tb_signal->rowCount();
            for(int i=0;i<rowcount;i++) {
                QLineSeries *tmp = new QLineSeries(this);
                auto color = color_group_.at(i);
                tmp->setColor(color);
                tmp->setName(QString::fromStdString(signal_name_list_.at(i)));
                series_group_.push_back(tmp);
                ui->chart->chart()->addSeries(tmp);
                tmp->attachAxis(axisX_);
                tmp->attachAxis(axisY_);
                tmp->hide();
            }
            qDebug() <<"series size: " << series_group_.size();
        }
        else {
            QMessageBox::warning(this,"提示", "打开文件错误", QMessageBox::Close, QMessageBox::Close);
        }
    });

    // btn_add_chart
    connect(ui->btn_add_chart, &QPushButton::clicked, this, [=](){
        CreateNewChart();
    });
}

void MonitorDialog::update_table_content(int number)
{
    ui->tb_signal->clearContents();
    qDebug() << ui->chart->chart()->series().size();
    ui->chart->chart()->removeAllSeries();

    qDebug() << ui->chart->chart()->series().size();
    series_group_.clear();
    ui->tb_signal->setRowCount(number);

    y_range_.clear();
    std::vector<float> tmp{0.0, 0.0};
    for(int i=0;i<number;i++) {
        y_range_.push_back(tmp);
    }
    qDebug() << "y_range_: " << y_range_.size();

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
        color_group_.push_back(itemLine->textColor());

        // name
        QTableWidgetItem *it = new QTableWidgetItem("");
        QString str = signal_name_list_.at(i).c_str();
        it->setText(str);
        ui->tb_signal->setItem(i,2,it);
    }
}

void MonitorDialog::update_table_content2(int number)
{
    ui->tb_signal->clearContents();
    for(auto it=chartview_list_.rbegin();it!=chartview_list_.rend();it++) {
        (*it)->chart()->removeAllSeries();
        (*it)->close();
    }
    CreateNewChart();
    current_chartview_ = chartview_list_.front();

    series_group_.clear();
    ui->tb_signal->setRowCount(number);

    y_range_.clear();
    std::vector<float> tmp{0.0, 0.0};
    for(int i=0;i<number;i++) {
        y_range_.push_back(tmp);
    }
    qDebug() << "y_range_: " << y_range_.size();

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
        color_group_.push_back(itemLine->textColor());

        // name
        QTableWidgetItem *it = new QTableWidgetItem("");
        QString str = signal_name_list_.at(i).c_str();
        it->setText(str);
        ui->tb_signal->setItem(i,2,it);
    }
}

void MonitorDialog::on_mouseMovePoint(QPoint point)
{
    QPointF pt = ui->chart->chart()->mapToValue(point);
    //    qDebug() << pt.x() << " " << pt.y();
}

void MonitorDialog::CreateNewChart()
{
    QChart *chart = new QChart();
    chart->setParent(this);
//    chart->setTheme(QChart::ChartThemeBrownSand);
    chart->layout()->setContentsMargins(2,2,2,2);
    chart->setMargins(QMargins(0,0,0,0));
    QValueAxis *axisX = new QValueAxis(this);
    QValueAxis *axisY = new QValueAxis(this);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    axisX->setRange(0, AXIS_X_SIZE_DEFAULT);
    axisX->setGridLineVisible(1);
    axisY->setRange(-10,10);
    axisY->setGridLineVisible(1);
    axisY->setTitleText("Value");

    AiccChartView *chartview = new AiccChartView(this);
    chartview->setChart(chart);
    chart->setAnimationOptions(QChart::NoAnimation);
    ui->verticalLayout_2->addWidget(chartview);
//    chartview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);//

    chartview_list_.emplace_back(std::move(chartview));
    qDebug() << "chartview: " << chartview_list_.size();
    connect(chartview, &AiccChartView::sendObjectAddr, this, [=](size_t addr){
        for(auto it=chartview_list_.begin();it!=chartview_list_.end();it++) {
            if( (*it) == (AiccChartView*)addr) {
                qDebug() << "chartview erase: " << *it;
                it = chartview_list_.erase(it);
                break;
            }
        }
    });
    connect(chartview, &AiccChartView::currentSelect, this, [=](size_t addr){
        current_chartview_ = (AiccChartView*)addr;
    });

}
