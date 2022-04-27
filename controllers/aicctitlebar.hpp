#ifndef AICCTITLEBAR_H
#define AICCTITLEBAR_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QMouseEvent>
#include <QFrame>
#include <QRect>

class AICCTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit AICCTitleBar(QWidget *parent = nullptr){
        // 不继承父组件的背景色
        setAutoFillBackground(true);
        // 使用 Highlight 作为背景色
        setBackgroundRole(QPalette::Highlight);

        minimize = new QToolButton(this);
        maximize = new QToolButton(this);
        close= new QToolButton(this);

        // 设置按钮图像的样式
        QPixmap pix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
        close->setIcon(pix);

        maxPix = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);
        maximize->setIcon(maxPix);

        pix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
        minimize->setIcon(pix);

        restorePix = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);

        minimize->setMinimumHeight(20);
        close->setMinimumHeight(20);
        maximize->setMinimumHeight(20);

        QLabel *label = new QLabel(this);
        label->setText("Window Title");
        parent->setWindowTitle("Window Title");

        QHBoxLayout *hbox = new QHBoxLayout(this);

        hbox->addWidget(label);
        hbox->addWidget(minimize);
        hbox->addWidget(maximize);
        hbox->addWidget(close);

        hbox->insertStretch(1, 500);
        hbox->setSpacing(0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        maxNormal = false;

        connect(close, SIGNAL( clicked() ), parent, SLOT(close() ) );
        connect(minimize, SIGNAL( clicked() ), this, SLOT(showSmall() ) );
        connect(maximize, SIGNAL( clicked() ), this, SLOT(showMaxRestore() ) );
    }

    ~AICCTitleBar(){};

signals:



public slots:
    void showSmall()
    {
        parentWidget()->showMinimized();
    }

    void showMaxRestore()
    {
        if (maxNormal) {
            parentWidget()->showNormal();
            maxNormal = !maxNormal;
            maximize->setIcon(maxPix);
        } else {
            parentWidget()->showMaximized();
            maxNormal = !maxNormal;
            maximize->setIcon(restorePix);
        }
    }

protected:
    void mousePressEvent(QMouseEvent *me)
    {
        startPos = me->globalPos();
        clickPos = mapToParent(me->pos());
    }
    void mouseMoveEvent(QMouseEvent *me)
    {
        if (maxNormal)
            return;
        parentWidget()->move(me->globalPos() - clickPos);
    }

private:
    QToolButton *minimize;
    QToolButton *maximize;
    QToolButton *close;
    QPixmap restorePix, maxPix;
    bool maxNormal;
    QPoint startPos;
    QPoint clickPos;


};

//class AICCFrame : public QFrame
//{
//public:
//    AICCFrame()
//    {
//        mouseDown = false;
//        setFrameShap(Panel);

//        // 设置无边框窗口
//        // 这会导致该窗口无法改变大小或移动
//        setWindowFlags(Qt::FramelessWindowHint);
//        setMouseTracking(true);

//        titleBar = new AICCTitleBar(this);

//        content = new QWidget(this);

//        QVBoxLayout *vbox = new QVBoxLayout(this);
//        vbox->addWidget(titleBar);
//        vbox->setMargin(0);
//        vbox->setSpacing(0);

//        QVBoxLayout *layout = new QVBoxLayout;
//        layout->addWidget(content);
//        layout->setMargin(5);
//        layout->setSpacing(0);
//        vbox->addLayout(layout);
//    }

//    // 通过 getter 允许外界访问 frame 的 content 区域
//    // 其它子组件应该添加到这里
//    QWidget *contentWidget() const { return content; }

//    AICCTitleBar *titleBar() const { return titleBar; }

//    void mousePressEvent(QMouseEvent *e)
//    {
//        oldPos = e->pos();
//        mouseDown = e->button() == Qt::LeftButton;
//    }

//    void mouseMoveEvent(QMouseEvent *e)
//    {
//        int x = e->x();
//        int y = e->y();

//        if (mouseDown) {
//            int dx = x - oldPos.x();
//            int dy = y - oldPos.y();

//            QRect g = geometry();

//            if (left)
//                g.setLeft(g.left() + dx);
//            if (right)
//                g.setRight(g.right() + dx);
//            if (bottom)
//                g.setBottom(g.bottom() + dy);

//            setGeometry(g);

//            oldPos = QPoint(!left ? e->x() : oldPos.x(), e->y());
//        } else {
//            QRect r = rect();
//            left = qAbs(x - r.left()) <= 5;
//            right = qAbs(x - r.right()) <= 5;
//            bottom = qAbs(y - r.bottom()) <= 5;
//            bool hor = left | right;

//            if (hor && bottom) {
//                if (left)
//                    setCursor(Qt::SizeBDiagCursor);
//                else
//                    setCursor(Qt::SizeFDiagCursor);
//            } else if (hor) {
//                setCursor(Qt::SizeHorCursor);
//            } else if (bottom) {
//                setCursor(Qt::SizeVerCursor);
//            } else {
//                setCursor(Qt::ArrowCursor);
//            }
//        }
//    }

//    void mouseReleaseEvent(QMouseEvent *e)
//    {
//        mouseDown = false;
//    }

//private:
//    AICCTitleBar *titleBar;
//    QWidget *content;
//    QPoint oldPos;
//    bool mouseDown;
//    bool left, right, bottom;
//};

#endif // AICCTITLEBAR_H
