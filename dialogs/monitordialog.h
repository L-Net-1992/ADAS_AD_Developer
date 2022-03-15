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

using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE


class MonitorDialog : public QDialog
{
    Q_OBJECT

public:
    MonitorDialog(QWidget *parent = nullptr);
    ~MonitorDialog();

    QCheckBox * getCheckBox(int row, int column);

    void init_button();
    void init_chart();
    void init_table();
    void update_table_content(int number);
    void update_table_content2(int number);

private slots:
    void on_mouseMovePoint(QPoint point);

private:
    void CreateNewChart();

private:

    Ui::Dialog *ui;
    QValueAxis *axisX_;
    QValueAxis *axisY_;
    QTimer *timer1;
    QTimer *timer2 ;
//    QLineSeries *myseries;
    std::vector<QColor> color_group_;           // 保存信号颜色
    std::vector<QLineSeries*> series_group_;    // 保存信号series
    std::vector<std::string> signal_name_list_; // 保存信号名称清单
    std::map<std::string, std::vector<float>> signals_dataset_; // 保存信号数据
    std::vector<AiccChartView*> chartview_list_;
    AiccChartView *current_chartview_;
    QValueAxis *current_axisX_;
    QValueAxis *current_axisY_;

    std::vector<std::vector<float>> y_range_;   //保存y轴最大最小值<min,max>
    unsigned int x_index_ = 0;
    size_t signal_num_ = 0;     // 信号数据长度
    bool replay_running_ = 0;
};
#endif // DIALOG_H
