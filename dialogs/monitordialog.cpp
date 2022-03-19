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

MonitorDialog::MonitorDialog(const QString ip,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , inspector(ip)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);

    data_model_ = new MonitorDataModel;
    InitTable();
    CreatNewChart(data_model_);
    connect(data_model_, &MonitorDataModel::SignalListEvent, this, &MonitorDialog::tableSignalUpdate);

//    Inspector inspector{"127.0.0.1"};

    //获得所有可监测变量，map中key为名字，value是否监测
    auto var_state = inspector.getVarWatchState();

    for(auto it=var_state.begin();it!=var_state.end();++it) {
        QString name = it.key();
        qDebug() << it.key();
        QColor qc = QColor::fromHsl(rand()%360,rand()%256,rand()%200);
        data_model_->addSignalList(name,qc);
    }


    //变量更新的signal，参数为变量名字和值
    QObject::connect(&inspector, &Inspector::varUpdated, [=](const QString & name, float value){
        std::cout << "var " << name.toStdString() << ": " << value << std::endl;
        QPointF data(x_index_,value);
        QString str = name;
        data_model_->addSignalData(str, data);
        if(data.y() > y_val_range_.at(0)) y_val_range_[0] = data.y(); // max
        if(data.y() < y_val_range_.at(1)) y_val_range_[1] = data.y(); // min

//        if(++current_number == signal_active_number_) { x_index_++; current_number=0;}
//        qDebug() << "x_index: " << x_index_ <<" signal_active_number_: " << signal_active_number_;

        for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
            chart_list_.value(it.key())->axisY()->setRange(y_val_range_.at(1)-2, y_val_range_.at(0)+2);
        }

    });
    // timer3
    timer3 = new QTimer(this);
    timer3->setInterval(50);
    connect(timer3, &QTimer::timeout, this, [=](){
        x_index_++;
        for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
            if(x_index_ > 50) {
                chart_list_.value(it.key())->axisX()->setRange(x_index_-50, x_index_);
            }
            chart_list_.value(it.key())->axisY()->setRange(y_val_range_.at(1)-2, y_val_range_.at(0)+2);

            auto tmp = data_model_->get_signal_list();
            for(int i=0;i<tmp.size();++i) {
                if(data_model_->getSignalCheckboxState(tmp.at(i))) {
                    series_group_.value(tmp.at(i))->setVisible(1);
                } else {
                    series_group_.value(tmp.at(i))->setVisible(0);
                }
            }
        }

    });
//    QObject::connect(mainWindow.ui->actionInspector, &QAction::triggered, [&inspector](){

//        //监控使用
//        //获得所有可监测变量，map中key为名字，value是否监测
//        auto var_state = inspector.getVarWatchState();
//        qDebug() << "vars: "<< var_state;
//        //设置是否监测变量，只有设置为true的变量，才会触发varUpdate
//        QMap<QString, bool> set_state;
//        set_state["task0.seq.out"] = true;
//        inspector.setVarWatchState(set_state);


//        //标定使用
//        //获得所有可标定参数的列表，map中key为名字，value为当前值
//        auto param_value = inspector.getParamValue();
//        qDebug() << "params: " << param_value;
//        //标定参数，通过map可以一次设置多个变量
//        QMap<QString, float> set_param;
//        set_param["task0.n4"] = 3.14f;
//        inspector.setParamValue(set_param);

//    });



    // timer0
    timer0 = new QTimer(this);
    timer0->setInterval(20);
    connect(timer0, &QTimer::timeout, this, [=](){
        if(!data_model_->signalCount()) {
            qDebug() << "no signal";
            return;
        }
        static qreal t=0,intv=0.1;
        qreal y4=5*qSin(t);
        qreal y5=5*qSin(t+1);
        qreal y6=5*qSin(t+2);
        qreal y1=8*qSin(t+3);
        qreal y2=8*qSin(t+4);
        qreal y3=8*qSin(t+5);
        QVector<qreal> val{y1,y2,y3,y4,y5,y6};
        for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
            auto tmp = data_model_->get_signal_list();
            for(int i=0;i<tmp.size();++i) {
                data_model_->addSignalData(tmp.at(i),QPointF(t,val.at(i)));
            }

            if(t > 50) {
                chart_list_.value(it.key())->axisX()->setRange(t-50, t);
            }
        }
        t+=intv;
    });

    // timer1 for test
    timer1 = new QTimer(this);
    timer1->setInterval(20);
    connect(timer1, &QTimer::timeout, this, &MonitorDialog::timeoutSlotTimer1);
//    timer1->start();

    // timer2 for replay
    timer2 = new QTimer(this);
    timer2->setInterval(20);
    connect(timer2, &QTimer::timeout, this, &MonitorDialog::timeoutSlotTimer2);


}

MonitorDialog::~MonitorDialog()
{
    delete ui;
}

void MonitorDialog::InitTable()
{
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    ui->tableWidget->setColumnWidth(2,150);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);

    QStringList headers;
    headers << QStringLiteral("选择") << QStringLiteral("图例") << QStringLiteral("信号") << QStringLiteral("值") ;
    ui->tableWidget->setHorizontalHeaderLabels(headers);
}

void MonitorDialog::InitModel()
{

}

void MonitorDialog::CreatNewChart(MonitorDataModel *model)
{
    QChart *chart = new QChart();
    chart->setParent(this);
    QChartView * chartview = new QChartView(this);
    chartview->setRenderHint(QPainter::Antialiasing);
    chartview->setChart(chart);

//    chart->setTheme(QChart::ChartThemeBrownSand);   // 主题颜色
    chart->layout()->setContentsMargins(2,2,2,2);   // 外边距
    chart->setMargins(QMargins(0,0,0,0));           // 內变距
    chart->setBackgroundRoundness(0);               // 边角直角

    QValueAxis *axisX = new QValueAxis(this);
    QValueAxis *axisY = new QValueAxis(this);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    axisX->setRange(0, 50);
    axisX->setTickCount(11);
    axisX->setGridLineVisible(0);
    axisY->setRange(-10,10);
    axisY->setGridLineVisible(1);
    axisY->setTitleText("Value");
    chart->setAnimationOptions(QChart::NoAnimation);

    QString obj_name = QString("newchart"+QString::number(chart_number++));
    chartview->setObjectName(obj_name);
    ui->verticalLayout_2->addWidget(chartview);

    chart_list_.insert(chartview->objectName(), chart);
    for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
        qDebug() << it.key() << " " << it.value();
    }

    for(int i=0;i<model->get_signal_list().size();i++) {
        QLineSeries *series = new QLineSeries(this);
        series->setName(model->get_signal_list().at(i));
        series->setPen(QPen(Qt::red,1,Qt::SolidLine));
        chart->addSeries(series);
        series->attachAxis(chart->axisX());
        series->attachAxis(chart->axisY());
    }
    connect(model, &MonitorDataModel::SignalListEvent, this, [=](QString name, QColor color){
        QLineSeries *series = new QLineSeries(this);
        series->setName(name);
//        series->setPen(QPen(Qt::red,1,Qt::SolidLine));
        series->setColor(color);
        chart->addSeries(series);
        series->attachAxis(chart->axisX());
        series->attachAxis(chart->axisY());

        series_group_[name] = series;
//        qDebug() << "series_group size: " << series_group_.size();
    });
    connect(model, &MonitorDataModel::SignalDataEvent, this, [=](QString name, QPointF data){
        series_group_.value(name)->append(data.x(),data.y());
    });
}

void MonitorDialog::tableSignalUpdate(QString signal, QColor color)
{
    auto rowcount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(rowcount + 1);

    //Checkbox
    QCheckBox *ckb = new QCheckBox(this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(ckb);
    hLayout->setMargin(0);
    hLayout->setAlignment(ckb,Qt::AlignCenter);
    QWidget *wckb = new QWidget(ui->tableWidget);
    wckb->setLayout(hLayout);
    ui->tableWidget->setCellWidget(rowcount,0,wckb);

    data_model_->setSignalCheckboxState(signal,Qt::Unchecked);

    // color
    QTableWidgetItem *itemLine = new QTableWidgetItem("▃▃▃▃▃");
    itemLine->setTextColor(color/*QColor(rand()%256,rand()%256,rand()%256)*/);
    ui->tableWidget->setItem(rowcount,1,itemLine);

    // name
    QTableWidgetItem *it = new QTableWidgetItem("");
    it->setText(signal);
    ui->tableWidget->setItem(rowcount,2,it);

    connect(ckb, &QCheckBox::clicked, this, [=](){
        qDebug() << "checkbox state" << ckb->checkState() << "signal " << signal;
        data_model_->setSignalCheckboxState(signal,ckb->checkState());

        //设置是否监测变量，只有设置为true的变量，才会触发varUpdate
        QMap<QString, bool> set_state;
        if(ckb->checkState() == Qt::Checked) {
            set_state[signal] = true;
            inspector.setVarWatchState(set_state);
            signal_active_number_++;
        } else {
            set_state[signal] = false;
            inspector.setVarWatchState(set_state);
            if(signal_active_number_ == 0) return;
            signal_active_number_--;
        }
    });
}

void MonitorDialog::timeoutSlotTimer1()
{
    static qreal t=0,intv=0.1;
    qreal y1=5*qSin(t);
    qreal y2=5*qSin(t+5);
    qreal y3=8*qCos(t);
    QVector<qreal> val{y1,y2,y3};

    for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
        auto tmp = chart_list_.value(it.key());
        for(int i=0;i<tmp->series().size();i++) {
            auto line1 =static_cast<QLineSeries*>(tmp->series().at(i));
            line1->append(t,val.at(i));
        }

        if(t > 50) {
            chart_list_.value(it.key())->axisX()->setRange(t-50, t);
        }
    }
    t+=intv;
}

void MonitorDialog::timeoutSlotTimer2()
{
    if(x_index_ >= signal_num_) {
        replay_running_ = 0;
    }

    for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
        auto tmp = data_model_->get_signal_list();

        for(int i=0;i<tmp.size();++i) {
            if(replay_running_) {
                auto data = data_model_->getSignalDataSet(tmp.at(i),x_index_);
                data_model_->addSignalData(tmp.at(i), data);
                if(data.y() > y_val_range_.at(0)) y_val_range_[0] = data.y(); // max
                if(data.y() < y_val_range_.at(1)) y_val_range_[1] = data.y(); // min
            }
            if(data_model_->getSignalCheckboxState(tmp.at(i))) {
                series_group_.value(tmp.at(i))->setVisible(1);
            } else {
                series_group_.value(tmp.at(i))->setVisible(0);
            }
        }

        if(replay_running_) {
            if(x_index_ > 50) {
                chart_list_.value(it.key())->axisX()->setRange(x_index_-50, x_index_);
            }
            // TODO: select the series 0 as y-axis range
            chart_list_.value(it.key())->axisY()->setRange(y_val_range_.at(1)-2, y_val_range_.at(0)+2);
            x_index_++;
        }
    }
}

void MonitorDialog::on_btn_replay_open_clicked()
{
    if(ui->btn_replay_start->text() == "stop") {
        QMessageBox::information(this, "提示", "请先关闭回放按键", QMessageBox::Close, QMessageBox::Close);
        return;
    }

    QString filename = QFileDialog::getOpenFileName(this, "打开一个文件", QApplication::applicationDirPath(), tr("HDF5 (*.h5 *.hdf5)"));
    if(filename.isEmpty()) return ;

    Hdf5Handle file;
    hid_t ret = file.open(filename.toStdString());
    if(ret) {
        // clear model
        data_model_->clearModel();

        while(ui->tableWidget->rowCount()) {
            ui->tableWidget->removeRow(0);
        }
        for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
            it.value()->removeAllSeries();
        }
        series_group_.clear();
        x_index_ = 0;
        for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
            chart_list_.value(it.key())->axisX()->setRange(0, 50);
        }

        // save signal list to model
        auto signalname = file.get_list("/Signal");
        for(int i=0;i<signalname.size();i++) {
            QColor qc = QColor::fromHsl(rand()%360,rand()%256,rand()%200);
            data_model_->addSignalList(QString::fromStdString(signalname.at(i)), qc);
        }

        // save data
        for(size_t i=0; i<signalname.size();i++) {
           std::vector<float> arr_float = file.get_data<float>(signalname.at(i), "/Signal");
           QVector<QPointF> tmp;
           for(size_t j=0;j<arr_float.size();j++) {
               tmp.push_back(QPointF(j,arr_float.at(j)));
           }
           data_model_->setSignalDataSet(QString::fromStdString(signalname.at(i)), tmp);
        }
         file.close();
         replay_loadfile =1;
         signal_num_ = data_model_->signalDataSize();
    }
    else {
        QMessageBox::information(this,"提示", "打开文件错误", QMessageBox::Close, QMessageBox::Close);
    }
}

void MonitorDialog::on_btn_monitor_record_clicked()
{
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
}

void MonitorDialog::on_btn_monitor_start_clicked()
{
    // 读取要监测的信号列表，创建信号
    timer3->start();

    if(0){
        QString name = "signal" + QString::number(line_number_++);
        QColor qc = QColor::fromHsl(rand()%360,rand()%256,rand()%200);
        data_model_->addSignalList(name,qc);
    }
}

void MonitorDialog::on_btn_monitor_stop_clicked()
{
    timer3->stop();

//    QMap<QString, bool> set_state;
//    for(auto it=series_group_.begin();it!=series_group_.end();++it) {
//        set_state[it.key()] = false;
//        inspector.setVarWatchState(set_state);
//    }
//    for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
//        it.value()->removeAllSeries();
//    }
//    current_number = 0;
//    signal_active_number_ = 0;
//    x_index_ = 0;

//    if(ui->btn_monitor_stop->text() == "on"){
//        ui->btn_monitor_stop->setText("stop");
//        timer0->start();
//        qDebug() << ui->btn_monitor_stop->text();
//    }
//    else {
//        ui->btn_monitor_stop->setText("on");
//        timer0->stop();
//        qDebug() << ui->btn_monitor_stop->text();
//    }

}

void MonitorDialog::on_btn_add_clicked()
{
    //    CreatNewChart(data_model_);
}

void MonitorDialog::on_btn_replay_start_clicked()
{
    if((replay_loadfile==0) || (data_model_->signalCount()==0)) return;

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
        timer2->start();

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
            // clear model
            data_model_->clearModel();
            while(ui->tableWidget->rowCount()) {
                ui->tableWidget->removeRow(0);
            }
            for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
                it.value()->removeAllSeries();
            }
            series_group_.clear();

            x_index_ = 0;
            for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
                chart_list_.value(it.key())->axisX()->setRange(0, 50);
            }
            timer2->stop();
            return;
        }
        replay_running_ = 0;
        timer2->stop();
    }
}

void MonitorDialog::closeEvent(QCloseEvent *e)
{
    if((replay_running_==1) || (monitor_running==1)) {
        QMessageBox::information(this, "提示", "停止运行按键", QMessageBox::Close, QMessageBox::Close);
        e->ignore();
        return;
    }
   auto button =  QMessageBox::information(this, "关闭窗口", "确定退出监测窗口", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
   if(button==QMessageBox::No)
    {
       e->ignore(); // 忽略退出信号，程序继续进行
    }
    else if(button==QMessageBox::Yes)
    {
       // clear model
       data_model_->clearModel();
       while(ui->tableWidget->rowCount()) {
           ui->tableWidget->removeRow(0);
       }
       for(auto it=chart_list_.begin();it!=chart_list_.end();it++) {
           it.value()->removeAllSeries();
       }
       series_group_.clear();
       x_index_=0;
       signal_num_=0;
       replay_running_=0;
       monitor_running = 0;
       replay_loadfile = 0;
       e->accept(); // 接受退出信号，程序退出
   }
}

void MonitorDialog::showEvent(QShowEvent *)
{

}
