#include "monitordialog.h"
#include "ui_monitordialog.h"
#include "hdf5/hdf5files_handle.h"
#include <QDateTime>
#include <QtDebug>
#include <QCheckBox>
#include <QValueAxis>
#include "monitor_files/monitor_chartview.h"

#define MONITOR_TIMER_MS   (100)
#define REPLAY_TIMER_MS    (100)
#define CHART_AXIS_X_RANGE (20)

using namespace utility;

MonitorDialog::MonitorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MonitorDialog)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_QuitOnClose, false);
    // 设在窗口属性
    setWindowTitle(tr("信号监测&回放"));
//    ui->tabWidget->setStyleSheet("QTabBar::tab { height: 30px; width: 120px; }");

    // 禁止按键功能
    ui->btn_add_chart->setVisible(false);
    ui->btn_add_chart->setEnabled(false);
    ui->btn_monitor_record->setEnabled(false);
    ui->btn_monitor_record_stop->setEnabled(false);
    ui->btn_monitor_start->setEnabled(false);
    ui->btn_monitor_stop->setEnabled(false);
    ui->lineEdit->setText("127.0.0.1");

    // 设置表头
    InitTableHeader(ui->monitortableWidget);
    InitTableHeader(ui->replaytableWidget);

    // 监测tab设置
    MonitorTabCreatNewChart("monitor_chart");
    connect(&monitor_, &Monitor::SignalListEvent, this, &MonitorDialog::MonitorTabTableSignalUpdate);
    monitor_timer_ = new QTimer(this);
    monitor_timer_->setInterval(MONITOR_TIMER_MS);
    connect(monitor_timer_, &QTimer::timeout, this, &MonitorDialog::timeoutSlotTimer3);
    connect(&monitor_, &Monitor::SignalDataEvent, this, &MonitorDialog::SaveSignalData);
    connect(&monitor_, &Monitor::SignalDataGroupEvent, this, &MonitorDialog::SaveSignalDataGroup);

    // 回放tab设置
    ReplayTabCreatNewChart("replay_chart");
    connect(&replay_, &Replay::SignalListEvent, this, &MonitorDialog::ReplayTabTableSignalUpdate);
    replay_timer_ = new QTimer(this);
    replay_timer_->setInterval(REPLAY_TIMER_MS);
    connect(replay_timer_, &QTimer::timeout, this, &MonitorDialog::timeoutSlotTimer2);

    // 保存记录

}

MonitorDialog::~MonitorDialog()
{
    delete ui;

    if(inspector_) {
        auto tmp = monitor_.GetSignalName();
        for(int i=0;i<tmp.size();i++) {
            if(monitor_.GetSignalCheckboxState(tmp.at(i)) == Qt::Checked) {
                QMap<QString, bool> set_state;
                set_state[tmp.at(i)] = false;
                inspector_->setVarWatchState(set_state);
            }
        }
        delete inspector_;
        inspector_ = nullptr;
    }

    replay_parameter_clear();

}

void MonitorDialog::InitTableHeader(QTableWidget *tableWidget)
{
    tableWidget->setColumnCount(4);
    tableWidget->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    tableWidget->setColumnWidth(2,150);
    tableWidget->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);

    QStringList headers;
    headers << QStringLiteral("选择") << QStringLiteral("图例") << QStringLiteral("信号") << QStringLiteral("值") ;
    tableWidget->setHorizontalHeaderLabels(headers);
}

void MonitorDialog::ReplayTabCreatNewChart(QString name)
{
    QChart *chart = new QChart();
    chart->setParent(this);
    r_chartview= new MonitorChartView(this);
    r_chartview->setRenderHint(QPainter::Antialiasing);
    r_chartview->setChart(chart);

//    chart->setTheme(QChart::ChartThemeBlueCerulean);   // 主题颜色
    chart->layout()->setContentsMargins(2,2,2,2);   // 外边距
    chart->setMargins(QMargins(0,0,0,0));           // 內变距
    chart->setBackgroundRoundness(0);               // 边角直角

    QValueAxis *axisX = new QValueAxis(this);
    QValueAxis *axisY = new QValueAxis(this);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    axisX->setRange(0, CHART_AXIS_X_RANGE);
    axisX->setTickCount(11);
    axisX->setMinorTickCount(4);
    axisX->setMinorGridLineVisible(0);
    axisX->setGridLineVisible(0);
    axisX->setTitleText("[S]");
    axisY->setRange(-50,50);
    axisY->setMinorTickCount(4);
    axisY->setMinorGridLineVisible(0);
    axisY->setGridLineVisible(1);
    axisY->setTitleText("[Value]");
    chart->setAnimationOptions(QChart::NoAnimation);

    r_chartview->setObjectName(name);
    ui->verticalLayout_6->addWidget(r_chartview);
    replay_chart_.insert(name, chart);

    connect(&replay_, &Replay::SignalListEvent, this, [=](QString name, QColor color){
        QLineSeries *series = new QLineSeries(this);
        series->setName(name);
        series->setColor(color);
        chart->addSeries(series);
        series->attachAxis(chart->axisX());
        series->attachAxis(chart->axisY());
        replay_series_[name] = series;
        series->setVisible(false);
        series->setUseOpenGL(true);
    });
    connect(&replay_, &Replay::SignalDataEvent, this, [=](QString name, QPointF data){
        replay_series_.value(name)->append(data.x(),data.y());
    });
}

void MonitorDialog::MonitorTabCreatNewChart(QString name)
{
    QChart *chart = new QChart();
    chart->setParent(this);
    m_chartview = new MonitorChartView(this);
    m_chartview->setRenderHint(QPainter::Antialiasing);
    m_chartview->setChart(chart);

//    chart->setTheme(QChart::ChartThemeBrownSand);   // 主题颜色
    chart->layout()->setContentsMargins(2,2,2,2);   // 外边距
    chart->setMargins(QMargins(0,0,0,0));           // 內变距
    chart->setBackgroundRoundness(0);               // 边角直角

    QValueAxis *axisX = new QValueAxis(this);
    QValueAxis *axisY = new QValueAxis(this);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    axisX->setRange(0, CHART_AXIS_X_RANGE);
    axisX->setTickCount(11);
    axisX->setMinorTickCount(4);
    axisX->setMinorGridLineVisible(0);
    axisX->setGridLineVisible(0);
    axisX->setTitleText("[S]");
    axisY->setRange(-50,50);
    axisY->setMinorTickCount(4);
    axisY->setMinorGridLineVisible(0);
    axisY->setGridLineVisible(1);
    axisY->setTitleText("[Value]");
    chart->setAnimationOptions(QChart::NoAnimation);

    m_chartview->setObjectName(name);
    ui->verticalLayout_5->addWidget(m_chartview);
    monitor_chart_.insert(name, chart);

    connect(&monitor_, &Monitor::SignalListEvent, this, [=](QString name, QColor color){
        QLineSeries *series = new QLineSeries(this);
        series->setName(name);
        series->setColor(color);
        chart->addSeries(series);
        series->attachAxis(chart->axisX());
        series->attachAxis(chart->axisY());
        monitor_series_[name] = series;
        series->setVisible(false);
        series->setUseOpenGL(true);
    });

    // 信号包含一个数据
    connect(&monitor_, &Monitor::SignalDataEvent, this, [=](QString name, QPointF data){
        monitor_series_.value(name)->append(data.x(),data.y());
    });

    // 信号包含一组数据
    connect(&monitor_, &Monitor::SignalDataGroupEvent, this, [=](QVector<QMap<QString,QPointF>> datas){
        qDebug() << "size: " << datas.size();
        for(auto it=datas.begin(); it!=datas.end(); ++it) {
            QString name = it->begin().key();
            QPointF data = it->begin().value();
            monitor_series_.value(name)->append(data.x(),data.y());
        }
    });
}

void MonitorDialog::ReplayTabTableSignalUpdate(QString signal, QColor color)
{
    auto rowcount = ui->replaytableWidget->rowCount();
    ui->replaytableWidget->setRowCount(rowcount + 1);

    //Checkbox
    QCheckBox *ckb = new QCheckBox(this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(ckb);
    hLayout->setMargin(0);
    hLayout->setAlignment(ckb,Qt::AlignCenter);
    QWidget *wckb = new QWidget(ui->replaytableWidget);
    wckb->setLayout(hLayout);
    ui->replaytableWidget->setCellWidget(rowcount,0,wckb);

    replay_.SetSignalCheckboxState(signal,Qt::Unchecked);

    // color
    QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
    itemLine->setTextColor(color/*QColor(rand()%256,rand()%256,rand()%256)*/);
    ui->replaytableWidget->setItem(rowcount,1,itemLine);

    // name
    QTableWidgetItem *it = new QTableWidgetItem("");
    it->setText(signal);
    ui->replaytableWidget->setItem(rowcount,2,it);

    connect(ckb, &QCheckBox::clicked, this, [=](){
        qDebug() << "checkbox state" << ckb->checkState() << "signal " << signal;
        replay_.SetSignalCheckboxState(signal,ckb->checkState());
        replay_series_.value(signal)->setVisible(ckb->checkState());
    });
}

void MonitorDialog::MonitorTabTableSignalUpdate(QString signal, QColor color)
{
    auto rowcount = ui->monitortableWidget->rowCount();
    ui->monitortableWidget->setRowCount(rowcount+1);

    //Checkbox
    QCheckBox *ckb = new QCheckBox(this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(ckb);
    hLayout->setMargin(0);
    hLayout->setAlignment(ckb,Qt::AlignCenter);
    QWidget *wckb = new QWidget(ui->monitortableWidget);
    wckb->setLayout(hLayout);
    ui->monitortableWidget->setCellWidget(rowcount,0,wckb);

    // color
    QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
    itemLine->setTextColor(color/*QColor(rand()%256,rand()%256,rand()%256)*/);
    ui->monitortableWidget->setItem(rowcount,1,itemLine);

    // name
    QTableWidgetItem *it = new QTableWidgetItem("");
    it->setText(signal);
    ui->monitortableWidget->setItem(rowcount,2,it);

    connect(ckb, &QCheckBox::clicked, this, [=](){
        qDebug() << "checkbox state" << ckb->checkState() << "signal " << signal;
        monitor_.SetSignalCheckboxState(signal,ckb->checkState());

        monitor_series_.value(signal)->setVisible(ckb->checkState());
        //设置是否监测变量，只有设置为true的变量，才会触发varUpdate
        if(inspector_ != nullptr) {
            QMap<QString, bool> set_state;
            if(ckb->checkState() == Qt::Checked) {
                set_state[signal] = true;
                inspector_->setVarWatchState(set_state);
            } else {
                set_state[signal] = false;
                inspector_->setVarWatchState(set_state);
            }
        }
    });
    connect(ui->checkBox, &QCheckBox::clicked, this, [=](){
        qDebug() << ui->checkBox->checkState();
        if(ui->checkBox->checkState() == Qt::Checked) {
            ckb->setCheckState(Qt::Checked);
            emit ckb->clicked(1);
        }
        else {
            ckb->setCheckState(Qt::Unchecked);
            emit ckb->clicked(0);
        }
    });
}

// 回放坐标轴移动定时器
void MonitorDialog::timeoutSlotTimer2()
{
    double second = (replay_axis_x_++)/10;
    QValueAxis *axisX = static_cast<QValueAxis*>(r_chartview->chart()->axisX());
    const double xMin = axisX->min();
    const double xMax = axisX->max();

    if(second>xMax) {
        double dis = second-xMax;
        axisX->setRange(xMin+dis, xMax+dis);
    }

}

// 监测坐标轴移动定时器
void MonitorDialog::timeoutSlotTimer3()
{
    monitor_axis_x_++;
    double second = monitor_axis_x_/10;

    QValueAxis *axisX = static_cast<QValueAxis*>(m_chartview->chart()->axisX());
    const double xMin = axisX->min();
    const double xMax = axisX->max();

    if(second>xMax) {
        double dis = second-xMax;
        axisX->setRange(xMin+dis, xMax+dis);
    }
}

void MonitorDialog::SaveSignalData(QString name, QPointF data)
{
    if(record_running_ == true) {
        QPointF tmp(data.x()-record_start_,data.y());
        record_data_[name].push_back(tmp);
    }
}

void MonitorDialog::SaveSignalDataGroup(QVector<QMap<QString, QPointF> > datas)
{
    if(record_running_ == true) {
        for(auto it=datas.begin();it!=datas.end();++it) {
            QString name = it->begin().key();
            QPointF data = it->begin().value();
            QPointF tmp(data.x()-record_start_+0.05,data.y());
            record_data_[name].push_back(tmp);
        }
    }
}

// 窗口关闭按钮事件
//void MonitorDialog::closeEvent(QCloseEvent *e)
//{
//    if((replay_running_==1) || (monitor_running==1)) {
//        QMessageBox::information(this, "提示", "停止运行按键", QMessageBox::Close, QMessageBox::Close);
//        e->ignore();
//        return;
//    }
//   auto button =  QMessageBox::information(this, "关闭窗口", "确定退出监测窗口", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
//   if(button==QMessageBox::No)
//    {
//       e->ignore(); // 忽略退出信号，程序继续进行
//    }
//    else if(button==QMessageBox::Yes)
//    {
//       // clear model
//       data_model_->clearModel();
//       while(ui->tableWidget->rowCount()) {
//           ui->tableWidget->removeRow(0);
//       }
//       for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
//           it.value()->removeAllSeries();
//       }
//       series_group_.clear();
//       x_index_=0;
//       signal_num_=0;
//       replay_running_=0;
//       monitor_running = 0;
//       replay_loadfile = 0;
//       e->accept(); // 接受退出信号，程序退出
//    }
//}



void MonitorDialog::on_lineEdit_returnPressed()
{
    qDebug()<< ui->lineEdit->text();
}

// 设备连接按键
void MonitorDialog::on_btn_device_connect_clicked()
{
    if(ui->btn_device_connect->text() == "连接")
    {
        QString deviceip = ui->lineEdit->text();
        try {
            inspector_ = new Inspector(deviceip);
        }  catch (...) {
            qDebug() << "Inspector wrong";
            return;
        }

        //变量更新的signal，参数为变量名字和值
        QObject::connect(inspector_, &Inspector::varUpdated, this, [=](const QString & name, float value){
            if(monitor_running_) {
                auto x = static_cast<double>(timer_measure_.elapsed())/1000;
                QPointF data(x,value);
                QString str = name;

                // 复位延时定时器
                if(!tmp_delay_.isValid()) {
                    tmp_delay_.restart();
                }

                // 缓存信号值到缓冲区
                QMap<QString,QPointF> tmp;
                tmp[str] = data;
                tmp_values_.push_back(tmp);

                // 延时50ms或者缓冲区数据大于100个，则发送信号给chart显示
                if((tmp_delay_.elapsed()>=50) || (tmp_values_.size()>=100)) {
                    qDebug() << "elapsed: " << tmp_delay_.elapsed() << " | " << tmp_values_.size();
                    monitor_.SendSignalDataGroup(tmp_values_);
                    tmp_values_.clear();
                    tmp_delay_.invalidate();
                }
            }
        });

        auto var_state = inspector_->getVarWatchState();
        if(var_state.size()) {
            // 更新table
            while(ui->monitortableWidget->rowCount()) {
                ui->monitortableWidget->removeRow(0);
            }
            for(auto it=monitor_chart_.begin();it!=monitor_chart_.end();it++) {
                it.value()->removeAllSeries();
            }

            for(auto it=var_state.begin();it!=var_state.end();++it) {
                QColor qc = QColor::fromHsl(rand()%360,rand()%256,rand()%200);
                monitor_.AddSignalList(it.key(), qc);
            }
           ui->btn_device_connect->setText(tr("断开"));
           ui->lineEdit->setEnabled(false);
           // 使能按键功能 tab1
           ui->btn_monitor_start->setEnabled(true);
        }
        else{
           qDebug() << "inspector woring";
        }
    }
    else {
        ui->btn_device_connect->setText(tr("连接"));

        auto tmp = monitor_.GetSignalName();
        for(int i=0;i<tmp.size();i++) {
            if(monitor_.GetSignalCheckboxState(tmp.at(i)) == Qt::Checked) {
                QMap<QString, bool> set_state;
                set_state[tmp.at(i)] = false;
                inspector_->setVarWatchState(set_state);
            }
        }
        delete inspector_;
        inspector_ = nullptr;

        ui->lineEdit->setEnabled(true);
        // 禁止按键功能
        ui->btn_add_chart->setEnabled(false);
        ui->btn_monitor_record->setEnabled(false);
        ui->btn_monitor_record_stop->setEnabled(false);
        ui->btn_monitor_start->setEnabled(false);
        ui->btn_monitor_stop->setEnabled(false);
    }
}

// 回放开始停止按键
void MonitorDialog::on_btn_replay_start_clicked()
{
    if(replay_.SignalListSize() == 0) return;

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
        replay_timer_->start();
        emit replay_signal(true);
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


        replay_running_ = 0;
        replay_timer_->stop();
        emit replay_signal(false);
    }
}

// 回放打开文件按键
void MonitorDialog::on_btn_replay_open_clicked()
{
    if(replay_running_) {
        QMessageBox msg(QMessageBox::NoIcon, "提示", "请先关闭回放按键", QMessageBox::Close, NULL);
        msg.exec();
        return;
    }
    QString filename = QFileDialog::getOpenFileName(this, "打开一个文件", QApplication::applicationDirPath(), tr("HDF5 (*.h5 *.hdf5)"));
    if(filename.isEmpty()) return ;

    Hdf5Handle file;
    hid_t ret = file.open(filename.toStdString());
    if(ret) {
        // clear buffer
        replay_.Clear();
        while(ui->replaytableWidget->rowCount()) {
            ui->replaytableWidget->removeRow(0);
        }
        for(auto it=replay_chart_.begin();it!=replay_chart_.end();it++) {
            it.value()->removeAllSeries();
        }
        replay_series_.clear();
        replay_axis_x_ = 0;
        for(auto it=replay_chart_.begin();it!=replay_chart_.end();it++) {
            replay_chart_.value(it.key())->axisX()->setRange(0, CHART_AXIS_X_RANGE);
        }
        replay_data_.clear();

        // 清除上个文件指针
        replay_parameter_clear();

        // save signal list to model
        auto signalname = file.get_list("/Signal");
        for(size_t i=0;i<signalname.size();i++) {
            QColor qc = QColor::fromHsl(rand()%360,rand()%256,rand()%200);
            replay_.AddSignalList(QString::fromStdString(signalname.at(i)), qc);
        }
        // save data
        for(size_t i=0; i<signalname.size();i++) {
            typedef struct {
                double x;
                double y;
            } s1_t;
            hid_t s1_tid;
            s1_tid = H5Tcreate(H5T_COMPOUND, sizeof(s1_t));
            H5Tinsert(s1_tid, "X", HOFFSET(s1_t, x), H5T_NATIVE_DOUBLE);
            H5Tinsert(s1_tid, "Y", HOFFSET(s1_t, y), H5T_NATIVE_DOUBLE);

            QVector<s1_t> ret;
            hid_t group_id = H5Gopen2(file.get_file_id(), "/Signal", H5P_DEFAULT);

            hid_t dataset_id = H5Dopen2(group_id, signalname.at(i).c_str(), H5P_DEFAULT);
            hid_t dataspace_id = H5Dget_space(dataset_id);
            int rank = H5Sget_simple_extent_ndims(dataspace_id);

            hsize_t dims_out[]={0};
            H5Sget_simple_extent_dims(dataspace_id, dims_out, NULL);
            ret.resize(dims_out[0]*rank);
            H5Dread(dataset_id, s1_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, ret.data());

            // close
            H5Sclose(dataspace_id);
            H5Dclose(dataset_id);

            // close group
            H5Gclose(group_id);

            QVector<QPointF> tmp ;
            for(auto it=ret.begin();it!=ret.end();++it) {
                QPointF val(it->x,it->y);
                tmp.push_back(val);
            }
            replay_data_[ QString::fromStdString(signalname.at(i))] = tmp;
        }
        file.close();

        // 文件回放
        for(auto it = replay_data_.begin();it!=replay_data_.end();++it){
            auto name = it.key();
            auto data = it.value();
            if(data.size()) {
                auto tmp = new SignalTimer(name,data,this);
                connect(this, &MonitorDialog::replay_signal, tmp, &SignalTimer::Start);
                connect(tmp, &SignalTimer::Send, this, [=](QString name, QPointF data){
                        replay_.SendSignalData(name,data);
                });
                connect(tmp, &SignalTimer::Complate, this, [=](int size){
                    if(size == max_sig_size) {
                        replay_timer_->stop();
                    }
                });
                replay_signal_timer_.push_back(std::move(tmp));
            }
            if(data.size()>max_sig_size) {
                max_sig_size = data.size();
            }
        }

    }
    else {
        QMessageBox::information(this,"提示", "打开文件错误", QMessageBox::Close, QMessageBox::Close);
    }
}

// 监测开始按键
void MonitorDialog::on_btn_monitor_start_clicked()
{
    auto tmp = monitor_.GetSignalName();
    int count=0;
    for(int i=0;i<tmp.size();i++) {
        if(monitor_.GetSignalCheckboxState(tmp.at(i)) == Qt::Checked)
            count++;
    }
    if(0 == count) return;

    ui->btn_monitor_start->setEnabled(false); ui->btn_monitor_start->setCheckable(false);
    ui->btn_monitor_stop->setEnabled(true);
    ui->btn_monitor_record->setEnabled(true);

    m_chartview->chart()->axisX()->setRange(0,CHART_AXIS_X_RANGE);
    for(int it=0;it<m_chartview->chart()->series().count();++it)
    {
        auto tmp = static_cast<QLineSeries*>(m_chartview->chart()->series().at(it));
        tmp->clear();
    }

    monitor_timer_->start();
    timer_measure_.restart();
    monitor_axis_x_ = 0;
    tmp_values_.clear();
    tmp_delay_.invalidate();
    monitor_running_=true;
    qDebug() << "monitor start";
}

// 监测停止按键
void MonitorDialog::on_btn_monitor_stop_clicked()
{
    if(!monitor_running_) return;

    monitor_timer_->stop();
    ui->btn_monitor_start->setEnabled(true);
    ui->btn_monitor_stop->setEnabled(false);
    ui->btn_monitor_record->setEnabled(false);

    if(record_running_) {
        record_running_ = false;
        ui->btn_monitor_record_stop->setEnabled(false);

        end_time_  = QString::number(QDateTime::currentMSecsSinceEpoch()).toULong();

        if(!record_file_name_.empty()) {
            qDebug() << "file name : " << QString::fromStdString(record_file_name_);

            QString dir_str = "log";
            QDir dir;
            if(!dir.exists(dir_str)) {
                dir.mkpath(dir_str);
            }

            std::string path = dir_str.toStdString() + "/" + record_file_name_;
            Hdf5Handle new_file;
            new_file.creat(path);

            typedef struct {
                double x;
                double y;
            } s1_t;
            hid_t s1_tid;
            s1_tid = H5Tcreate(H5T_COMPOUND, sizeof(s1_t));
            H5Tinsert(s1_tid, "X", HOFFSET(s1_t, x), H5T_NATIVE_DOUBLE);
            H5Tinsert(s1_tid, "Y", HOFFSET(s1_t, y), H5T_NATIVE_DOUBLE);

            // Signal
            for(auto it=record_data_.begin();it!=record_data_.end();++it) {
                hid_t group_id = H5Gopen2(new_file.get_file_id(), "/Signal", H5P_DEFAULT);
                int rank = 1;
                hsize_t dims[] = {static_cast<hsize_t>(it.value().size())};
                hid_t dataspace_id = H5Screate_simple(rank, dims, NULL);
                hid_t dataset_id = H5Dcreate2(group_id, it.key().toStdString().c_str(), s1_tid ,dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                herr_t status = H5Dwrite(dataset_id, s1_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, it.value().begin());

                H5Sclose(dataspace_id);
                H5Dclose(dataset_id);
                H5Gclose(group_id);
            }

            // Header
            {
                hid_t group_id = H5Gopen2(new_file.get_file_id(), "/Header", H5P_DEFAULT);
                int rank = 1;
                hsize_t dims[] = {1};
                hid_t dataspace_id = H5Screate_simple(rank, dims, NULL);
                hid_t dataset_id = H5Dcreate2(group_id, "start_time", H5T_NATIVE_ULONG ,dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                herr_t status = H5Dwrite(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &start_time_);

                H5Sclose(dataspace_id);
                H5Dclose(dataset_id);
                H5Gclose(group_id);
            }
            {
                hid_t group_id = H5Gopen2(new_file.get_file_id(), "/Header", H5P_DEFAULT);
                int rank = 1;
                hsize_t dims[] = {1};
                hid_t dataspace_id = H5Screate_simple(rank, dims, NULL);
                hid_t dataset_id = H5Dcreate2(group_id, "end_time", H5T_NATIVE_ULONG ,dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                herr_t status = H5Dwrite(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &end_time_);

                H5Sclose(dataspace_id);
                H5Dclose(dataset_id);
                H5Gclose(group_id);
            }

            if(new_file.get_file_id()) {
                new_file.close();
            }

        }
        else {
            qDebug() << "file name is empty";
        }


        // clear record buffer
        record_data_.clear();

    }
    monitor_running_=false;
    qDebug() << "monitor stop";
}

// 记录开始
void MonitorDialog::on_btn_monitor_record_clicked()
{
    if((monitor_running_ == 1) && (!record_running_)) {
        ui->btn_monitor_record->setEnabled(false);
        ui->btn_monitor_record_stop->setEnabled(true);

        QDateTime datetime;
        QString timestr = datetime.currentDateTime().toString("yyyy_MM_dd_HH_mm_ss");
        record_file_name_ = "log_" + timestr.toStdString() + ".h5";

        start_time_ = datetime.currentMSecsSinceEpoch();
        record_start_ =  static_cast<double>(timer_measure_.elapsed())/1000;
        record_running_ = true;
    }
}

// 记录停止按键
void MonitorDialog::on_btn_monitor_record_stop_clicked()
{
    ui->btn_monitor_record_stop->setEnabled(false);
    ui->btn_monitor_record->setEnabled(true);
    record_running_ = false;

    end_time_  = QString::number(QDateTime::currentMSecsSinceEpoch()).toULong();

    if(!record_file_name_.empty()) {
        qDebug() << "file name : " << QString::fromStdString(record_file_name_);

        QString dir_str = "log";
        QDir dir;
        if(!dir.exists(dir_str)) {
            dir.mkpath(dir_str);
        }

        std::string path = dir_str.toStdString() + "/" + record_file_name_;
        Hdf5Handle new_file;
        new_file.creat(path);

        typedef struct {
            double x;
            double y;
        } s1_t;
        hid_t s1_tid;
        s1_tid = H5Tcreate(H5T_COMPOUND, sizeof(s1_t));
        H5Tinsert(s1_tid, "X", HOFFSET(s1_t, x), H5T_NATIVE_DOUBLE);
        H5Tinsert(s1_tid, "Y", HOFFSET(s1_t, y), H5T_NATIVE_DOUBLE);

        // Signal
        for(auto it=record_data_.begin();it!=record_data_.end();++it) {
            hid_t group_id = H5Gopen2(new_file.get_file_id(), "/Signal", H5P_DEFAULT);
            int rank = 1;
            hsize_t dims[] = {static_cast<hsize_t>(it.value().size())};
            hid_t dataspace_id = H5Screate_simple(rank, dims, NULL);
            hid_t dataset_id = H5Dcreate2(group_id, it.key().toStdString().c_str(), s1_tid ,dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            herr_t status = H5Dwrite(dataset_id, s1_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, it.value().begin());

            H5Sclose(dataspace_id);
            H5Dclose(dataset_id);
            H5Gclose(group_id);
        }

        // Header
        {
            hid_t group_id = H5Gopen2(new_file.get_file_id(), "/Header", H5P_DEFAULT);
            int rank = 1;
            hsize_t dims[] = {1};
            hid_t dataspace_id = H5Screate_simple(rank, dims, NULL);
            hid_t dataset_id = H5Dcreate2(group_id, "start_time", H5T_NATIVE_ULONG ,dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            herr_t status = H5Dwrite(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &start_time_);

            H5Sclose(dataspace_id);
            H5Dclose(dataset_id);
            H5Gclose(group_id);
        }
        {
            hid_t group_id = H5Gopen2(new_file.get_file_id(), "/Header", H5P_DEFAULT);
            int rank = 1;
            hsize_t dims[] = {1};
            hid_t dataspace_id = H5Screate_simple(rank, dims, NULL);
            hid_t dataset_id = H5Dcreate2(group_id, "end_time", H5T_NATIVE_ULONG ,dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            herr_t status = H5Dwrite(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &end_time_);

            H5Sclose(dataspace_id);
            H5Dclose(dataset_id);
            H5Gclose(group_id);
        }

        if(new_file.get_file_id()) {
            new_file.close();
        }

    }
    else {
        qDebug() << "file name is empty";
    }


    // clear record buffer
    record_data_.clear();

}

void MonitorDialog::replay_parameter_clear()
{
    // 释放回放信号指针
    max_sig_size = 0;
    for (auto iter=replay_signal_timer_.begin(); iter!=replay_signal_timer_.end(); ++iter)
    {
        if (*iter != nullptr)
        {
            delete (*iter);
            (*iter) = nullptr;
        }
    }
    replay_signal_timer_.clear();
    QVector<SignalTimer*> tmpSwapVector;
    tmpSwapVector.swap(replay_signal_timer_);
}

