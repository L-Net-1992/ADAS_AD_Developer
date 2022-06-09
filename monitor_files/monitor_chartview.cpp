#include "monitor_chartview.h"

MonitorChartView::MonitorChartView(QWidget *parent)
    : QChartView{parent}
{
    Init();
}

MonitorChartView::MonitorChartView(QChart *chart, QWidget *parent)
    : QChartView{chart, parent}
{

}

void MonitorChartView::Init()
{
    btn_close_ = new QToolButton(this);
    QPixmap pix_close = this->style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
    btn_close_->setIcon(pix_close);
    btn_close_->setGeometry(0, 0, 15, 15);
    btn_close_->setToolTip("关闭");
    btn_close_->setStyleSheet("color:black;border-radius：10px");
    connect(btn_close_, &QToolButton::clicked, this, [=](){
//        qDebug() << this;
        emit sendObjectAddr(size_t(this));
        this->close();
    });
    btn_close_->setVisible(false);

    this->setDragMode(QGraphicsView::RubberBandDrag);
    this->setMouseTracking(true);
}

MonitorChartView::~MonitorChartView()
{
//    qDebug("MonitorChartView----");
}

void MonitorChartView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        begin_point_ = event->pos();
    QChartView::mousePressEvent(event);
}

void MonitorChartView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint point = event->pos();
    emit mouseMovePoint(point);
    QChartView::mouseMoveEvent(event);
}

void MonitorChartView::mouseReleaseEvent(QMouseEvent *event)
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

void MonitorChartView::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Control:
        ctrl_key_ = 0;
        break;
    default:
        break;
    }
    QChartView::keyReleaseEvent(event);
}

void MonitorChartView::keyPressEvent(QKeyEvent *event)
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
        case Qt::Key_Control:
            ctrl_key_ = 1;
            break;
        default:
            QGraphicsView::keyPressEvent(event);
            break;
    }
    QChartView::keyPressEvent(event);
}

void MonitorChartView::wheelEvent(QWheelEvent *event)
{
    const QPoint curPos = event->pos();
    QPointF curVal = this->chart()->mapToValue(QPointF(curPos));
//qDebug() << "curPos: " << curPos << " curVal: " << curVal;
    const double factor = 1.5;
    if(ctrl_key_) {
        QValueAxis *axisY = static_cast<QValueAxis*>(this->chart()->axisY());
        const double yMin = axisY->min();
        const double yMax = axisY->max();
        const double yCentral = curVal.y();

        double bottomOffset;
        double topOffset;
        if(event->delta()>0) {
            bottomOffset = 1.0/factor * (yCentral -yMin);
            topOffset = 1.0/factor * (yMax - yCentral);
        }
        else {
            bottomOffset = 1.0*factor * (yCentral -yMin);
            topOffset = 1.0*factor * (yMax - yCentral);
        }
        this->chart()->axisY()->setRange(yCentral-bottomOffset, yCentral+topOffset);
    }
    else {
        QValueAxis *axisX = static_cast<QValueAxis*>(this->chart()->axisX());
        const double xMin = axisX->min();
        const double xMax = axisX->max();
        const double xCentral = curVal.x();

        double leftOffset;
        double rightOffset;
        if(event->delta()>0){
            leftOffset = 1.0/factor * (xCentral-xMin);
            rightOffset = 1.0/factor * (xMax - xCentral);
        }
        else {
            leftOffset = 1.0*factor * (xCentral-xMin);
            rightOffset = 1.0*factor * (xMax - xCentral);
        }
        this->chart()->axisX()->setRange(xCentral-leftOffset, xCentral+rightOffset);
    }
}

void MonitorChartView::focusInEvent(QFocusEvent *event)
{
    if(QEvent::FocusIn == event->type()) {
//        qDebug() << "focus in";
        QPalette p=QPalette();
        p.setColor(QPalette::Window, Qt::green);
        setPalette(p);
    }
}

void MonitorChartView::focusOutEvent(QFocusEvent *event)
{
    if(QEvent::FocusOut == event->type()) {
//        qDebug() << "focus out";
        QPalette p=QPalette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);
    }
}

