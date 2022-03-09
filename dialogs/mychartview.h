#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QtCharts>
QT_CHARTS_USE_NAMESPACE

class MyChartView : public QChartView
{
    Q_OBJECT
private:
    QPoint begin_point_;
    QPoint end_point_;

protected:
    void mousePressEvent(QMouseEvent *event); //鼠标左键按下
    void mouseMoveEvent(QMouseEvent *event); //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event); //鼠标释放左键
    void keyPressEvent(QKeyEvent *event); //按键事件

public:
    explicit MyChartView(QWidget *parent = 0);
    ~MyChartView();

signals:
    void mouseMovePoint(QPoint point); //鼠标移动信号，在mouseMoveEvent()事件中触发
};

#endif // MYCHARTVIEW_H
