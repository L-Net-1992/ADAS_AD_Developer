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
    void timeoutSlotTimer3();
    void SaveSignalData(QString name, QPointF data);
    void SaveSignalDataGroup(QVector<QMap<QString,QPointF>> datas);

protected:
    virtual void closeEvent(QCloseEvent *e) override;

private slots:
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
    void monitorStateReset();
    void recordStateReset();
    void ReplayParaReset();
private:
    Ui::MonitorDialog *ui;

private:
    Inspector *inspector_{nullptr};
    QElapsedTimer timer_measure_;  // 用于产生收到数据时的时刻值

    // Monitor参数定义
    Monitor monitor_;
    QTimer *monitor_timer_;
    MonitorChartView *m_chartview_;
    QMap<QString, QLineSeries*> monitor_series_;
    QMap<QString, QChart*> monitor_chart_;
    double monitor_axis_x_{0};
    bool monitor_running_{false};
    QVector<QMap<QString,QPointF>> tmp_values_; // 用于接收数据缓存用
    QElapsedTimer tmp_delay_;  // 用于接收数据缓存的50ms计时

    // Record参数定义
    Record record_;
    QMap<QString, QVector<QPointF>> record_data_;
    bool record_running_{false};
    double record_start_{0};

    // Replay参数定义
    Replay replay_;
    MonitorChartView *r_chartview;
    QMap<QString, QChart*> replay_chart_;
    bool replay_running_{0};
    QMap<QString, QLineSeries*> replay_series_;


};
#endif // MONITORDIALOG_H
