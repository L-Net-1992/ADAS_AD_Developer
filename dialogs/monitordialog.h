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
#include <QtCharts/QVXYModelMapper>
#include <QLineSeries>
#include <QSplineSeries>
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
//    void timeoutSlotTimer1();
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

private:
    Ui::MonitorDialog *ui;

private:
    Inspector *inspector_{nullptr};

    // Monitor参数定义
    Monitor monitor_;
    QTimer *monitor_timer_;
    MonitorChartView *m_chartview;
    QMap<QString, QLineSeries*> monitor_series_;
    QMap<QString, QChart*> monitor_chart_;
    int x_index1_ = 0;
    bool monitor_running_{false};
    QVector<float> y_val_range1_{-10.0,10.0};

    // Record参数定义
    bool record_running_=false;

    // Replay参数定义
    Replay replay;
    QTimer *replay_timer_;
    bool replay_running_ = 0;
    int x_index_ = 0;
    QVector<float> y_val_range_{-10.0,10.0};
    QMap<QString, QLineSeries*> replay_series_;
    QMap<QString, QChart*> replay_chart_;

};
#endif // MONITORDIALOG_H
