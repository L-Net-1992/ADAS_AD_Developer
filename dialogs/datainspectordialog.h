#ifndef DATAINSPECTORDIALOG_H
#define DATAINSPECTORDIALOG_H

#include <QDialog>
#include <QHeaderView>
#include <QCheckBox>
#include <QGraphicsLineItem>
#include <QChart>
#include <QChartView>
#include <QSplineSeries>
#include <QValueAxis>

namespace Ui {
class DataInspectorDialog;
}

using QtCharts::QChart;
using QtCharts::QChartView;
using QtCharts::QSplineSeries;
using QtCharts::QValueAxis;

class DataInspectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataInspectorDialog(QWidget *parent = nullptr);
    ~DataInspectorDialog();

private:
    void initTableWidget();
    void initCharts();
    void removeDockWidgetTopTitle();

private:
    Ui::DataInspectorDialog *ui;
};

#endif // DATAINSPECTORDIALOG_H
