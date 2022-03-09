#include "mychartview.h"
#include <QChartView>

void MyChartView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        begin_point_ = event->pos();
    QChartView::mousePressEvent(event);
}

void MyChartView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint point = event->pos();

    emit mouseMovePoint(point);
    QChartView::mouseMoveEvent(event);
}

void MyChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        end_point_ = event->pos();
        QRectF rec;
        rec.setTopLeft(this->begin_point_);
        rec.setBottomRight(this->end_point_);
        this->chart()->zoomIn(rec);
    }
    else if(event->button() == Qt::RightButton) {
        this->chart()->zoomReset();
    }
    QChartView::mouseReleaseEvent(event);
}

void MyChartView::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Plus:
            chart()->zoom(1.2);
            break;
        case Qt::Key_Minus:
            chart()->zoom(0.8);
            break;
        case Qt::Key_Left:
            chart()->scroll(10, 0);
            break;
        case Qt::Key_Right:
            chart()->scroll(-10, 0);
            break;
        case Qt::Key_Up:
            chart()->scroll(0, -10);
            break;
        case Qt::Key_Down:
            chart()->scroll(0, 10);
            break;
        case Qt::Key_PageUp:
            chart()->scroll(0, 50);
            break;
        case Qt::Key_PageDown:
            chart()->scroll(0, -50);
            break;
        case Qt::Key_Home:
            chart()->zoomReset();
            break;
        default:
            QGraphicsView::keyPressEvent(event);
            break;
    }
}

MyChartView::MyChartView(QWidget *parent) : QChartView(parent)
{
    this->setDragMode(QGraphicsView::RubberBandDrag);
    this->setMouseTracking(true);
}

MyChartView::~MyChartView()
{

}


