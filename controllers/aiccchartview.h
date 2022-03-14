#ifndef AICCCHARTVIEW_H
#define AICCCHARTVIEW_H

#include <QChartView>
#include <QtCharts>
#include <QPoint>

QT_CHARTS_USE_NAMESPACE

class AiccChartView : public QChartView
{
    Q_OBJECT
private:
    QPoint begin_point_;
    QPoint end_point_;

protected:
    void mousePressEvent(QMouseEvent *event) //鼠标左键按下
    {
        if(event->button() == Qt::LeftButton)
            begin_point_ = event->pos();
         QChartView::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) //鼠标移动
    {
        QPoint point = event->pos();
        emit mouseMovePoint(point);
        QChartView::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) //鼠标释放左键
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
    void keyPressEvent(QKeyEvent *event) //按键事件
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
    void wheelEvent(QWheelEvent *event)
    {
        //    if(event->delta() > 0) {
        //        qDebug() << "wheel +++";
        //    }
        //    else {
        //        qDebug() << "wheel ---";
        //    }
    }

    void focusInEvent(QFocusEvent *event)
    {
        if(QEvent::FocusIn == event->type()) {
            qDebug() << "focus in";
            QPalette p=QPalette();
            p.setColor(QPalette::Window, Qt::red);
            setPalette(p);

            emit currentSelect(size_t(this));
        }
    }

    void focusOutEvent(QFocusEvent *event)
    {
        if(QEvent::FocusOut == event->type()) {
            qDebug() << "focus out";
            QPalette p=QPalette();
            p.setColor(QPalette::Window, Qt::white);
            setPalette(p);
        }
    }

public:
    explicit AiccChartView(QWidget *parent = 0)
    {
        QToolButton *btn_close = new QToolButton(this);
        QPixmap pix_close = this->style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
        btn_close->setIcon(pix_close);
        btn_close->setGeometry(0, 0, 15, 15);
        btn_close->setToolTip("关闭");
        btn_close->setStyleSheet("color:black;border-radius：10px");
        connect(btn_close, &QToolButton::clicked, this, [=](){
            qDebug() << "chartview close: " << this;
            emit sendObjectAddr(size_t(this));
            this->close();
        });

        this->setDragMode(QGraphicsView::RubberBandDrag);
        this->setMouseTracking(true);
    }

    ~AiccChartView()
    {
        qDebug("AiccChartView----");
    }


signals:
    void mouseMovePoint(QPoint point); //鼠标移动信号，在mouseMoveEvent()事件中触发
    void sendObjectAddr(size_t addr);
    void currentSelect(size_t addr);

};


#endif
