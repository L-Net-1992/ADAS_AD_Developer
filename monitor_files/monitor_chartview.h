#ifndef MONITORCHARTVIEW_H
#define MONITORCHARTVIEW_H

#include <QChartView>
#include <QtCharts>
#include <QPoint>

class MonitorChartView : public QChartView
{
    Q_OBJECT
public:
    explicit MonitorChartView(QWidget *parent = nullptr);
    explicit MonitorChartView(QChart *chart, QWidget *parent = nullptr);
    void Init();
    ~MonitorChartView();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void keyPressEvent(QKeyEvent *event) ;
    void wheelEvent(QWheelEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
signals:
    void mouseMovePoint(QPoint point);
    void sendObjectAddr(size_t addr);
private:
    QPoint begin_point_;
    QPoint end_point_;
    bool ctrl_key_ = false;
    QToolButton *btn_close_;

};

#endif // MONITORCHARTVIEW_H
