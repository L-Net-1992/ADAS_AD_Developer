#ifndef MONITORDIALOG_H
#define MONITORDIALOG_H

#include <QDialog>
#include <QtCharts/QValueAxis>
#include <QCheckBox>
#include <QTimer>
#include <QList>
#include <QLineSeries>
#include <QtCharts/QSplineSeries>
#include "aiccchartview.h"

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QGraphicsLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QVXYModelMapper>
#include <QLineSeries>
#include <QTimer>
#include "monitordatamodel.hpp"
#include <QObject>

using namespace QtCharts;
QT_CHARTS_USE_NAMESPACE
#define     FixedColumnCount    4      //文件固定6列

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE


class MonitorDialog : public QDialog
{
    Q_OBJECT

public:
    MonitorDialog(QWidget *parent = nullptr);
    ~MonitorDialog();
    void InitTable();
    void InitModel();
    void CreatNewChart(MonitorDataModel *model);

private slots:
    void tableSignalUpdate(QString signal, QColor color);
    void timeoutSlotTimer1();
    void timeoutSlotTimer2();

    void on_btn_replay_open_clicked();

    void on_btn_monitor_record_clicked();

    void on_btn_monitor_start_clicked();

    void on_btn_monitor_stop_clicked();

    void on_btn_add_clicked();

    void on_btn_replay_start_clicked();

protected:
    virtual void closeEvent(QCloseEvent *e) override;

private:
    Ui::Dialog *ui;
    QTimer *timer0;
    QTimer *timer1;
    QTimer *timer2;

private:
    MonitorDataModel *data_model_;
    QMap<QString, QChart*> chart_list_;
    QMap<QString, QLineSeries*> series_group_;
    QMap<QString, QColor> color_group_;           // 保存信号颜色
    std::vector<std::vector<float>> y_range_;   //保存y轴最大最小值<min,max>
    QVector<float> y_val_range_{-10,10};
    unsigned int line_number_ = 0;
    unsigned int chart_number = 0;
    unsigned int x_index_ = 0;
    size_t signal_num_ = 0;     // 信号数据长度
    bool replay_running_ = 0;
    bool monitor_running = 0;
    bool replay_loadfile = 0;

};
#endif // DIALOG_H
