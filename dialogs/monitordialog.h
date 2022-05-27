#ifndef MONITORDIALOG_H
#define MONITORDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QGraphicsLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QTimer>
#include "monitor_files/monitor_signal.h"
#include "nodeparser/inspector.hpp"
#include "monitor_files/monitor_chartview.h"

using namespace monitor;
QT_CHARTS_USE_NAMESPACE

#define     FixedColumnCount    4      //文件固定4列

QT_BEGIN_NAMESPACE
namespace Ui { class MonitorDialog; }
QT_END_NAMESPACE

class MonitorDialog : public QDialog
{
    Q_OBJECT

public:
    MonitorDialog(QWidget *parent = nullptr);
    ~MonitorDialog();

    void InitTableHeader(QTableWidget *tableWidget);
    void ReplayTabCreatNewChart(QString name);
    void MonitorTabCreatNewChart(QString name);

private slots:
    void ReplayTabTableSignalUpdate(QString signal, QColor color);
    void MonitorTabTableSignalUpdate(QString signal, QColor color);
    void timeoutSlotTimer2();
    void timeoutSlotTimer3();
    void SaveSignalData(QString name, QPointF data);

//protected:
//    virtual void closeEvent(QCloseEvent *e) override;

private slots:
    void on_lineEdit_returnPressed();
    void on_btn_device_connect_clicked();
    void on_btn_replay_start_clicked();
    void on_btn_replay_open_clicked();
    void on_btn_monitor_start_clicked();
    void on_btn_monitor_stop_clicked();
    void on_btn_monitor_record_clicked();
    void on_btn_monitor_record_stop_clicked();

signals:
    void replay_signal(bool state);
private:
    void replay_parameter_clear();
private:
    Ui::MonitorDialog *ui;

private:
    Inspector *inspector_{nullptr};
    QElapsedTimer timer_measure_;

    // Monitor参数定义
    Monitor monitor_;
    QTimer *monitor_timer_;
    MonitorChartView *m_chartview;
    QMap<QString, QLineSeries*> monitor_series_;
    QMap<QString, QChart*> monitor_chart_;
    double monitor_axis_x_{0};
    bool monitor_running_{false};
    QMap<QString,QPointF> tmp_data_; // 保存临时数据，测试用
    QVector<QMap<QString,QPointF>> tmp_values_; // 测试用
    QElapsedTimer tmp_delay_;
    bool tmp_select_all_=0;

    // Record参数定义
    QMap<QString, QVector<QPointF>> record_data_;
    bool record_running_{false};
    unsigned long start_time_{0};
    unsigned long end_time_{0};
    std::string record_file_name_;
    double record_start_{0};

    // Replay参数定义
    Replay replay_;
    QTimer *replay_timer_;
    MonitorChartView *r_chartview;
    bool replay_running_{0};
    double replay_axis_x_{0};

    int max_sig_size{0};
    QVector<SignalTimer*> replay_signal_timer_;
    QMap<QString, QLineSeries*> replay_series_;
    QMap<QString, QChart*> replay_chart_;
    QMap<QString, QVector<QPointF>> replay_data_;

};
#endif // MONITORDIALOG_H
