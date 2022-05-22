#include "splashscreen.h"
#include <QPainter>
#include <QSysInfo>
#include <QThread>
#include <QTimer>
#include <QDebug>
SplashScreen::SplashScreen(const QPixmap &pixmap) : QSplashScreen(pixmap)
{
    init();
//    this->showMessage(QStringLiteral("正在加载资源文件，请稍后……"), Qt::AlignHCenter|Qt::AlignBottom, Qt::white);

}

SplashScreen::SplashScreen(const QPixmap &pixmap, const unsigned long ms) : QSplashScreen(pixmap)
{
    init();

    this->show();
    QThread::msleep(100);
    this->showMessage(QStringLiteral("正在加载资源文件，请稍后……"), Qt::AlignHCenter|Qt::AlignBottom, Qt::white);
    QThread::msleep(ms);
}

void SplashScreen::drawContents(QPainter *painter)
{
    painter->setFont(QFont("SimSun", 8));
    painter->setPen(QColor(213, 218, 220));
    painter->drawText(QPointF(20, 40), version_);

    painter->setFont(QFont("SimSun", 8));
    painter->setPen(QColor(213, 218, 220));
    painter->drawText(QPointF(20, 55), architecture_);

    painter->setFont(QFont("SimSun", 8));
    painter->setPen(QColor(213, 218, 220));
    painter->drawText(QPointF(20, 70), release_date_);

    // copyright
    QString copyright;
    copyright = "Copyright © 2022 AICC, All Rights Reserved.";
    painter->setFont(QFont("SimSun", 8));
    painter->setPen(QColor(213, 218, 220));
    QFontMetrics fm(painter->font());
    QRect rec = fm.boundingRect(copyright);
    painter->drawText((this->frameGeometry().width()-rec.width())/2, this->frameGeometry().height()-120, copyright);

    // logo
    QString logoname("AADS");
    painter->save();
    painter->setFont(QFont("SimSun", 30 ,75));
    QPen pen(QColor(138, 151, 123), 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    QFontMetrics f(painter->font());
    QRect r = f.boundingRect(logoname);
    painter->drawText((this->frameGeometry().width()-r.width())/2,this->frameGeometry().height()-150, logoname);
    painter->restore();

    // wait message
    QString msg = "正在加载资源文件，请稍后";
    painter->save();
    painter->setFont(QFont("SimSun", 8));
    painter->setPen(QColor(213, 218, 220));
    QFontMetrics fmsg(painter->font());
    QRect rmsg = fmsg.boundingRect(msg);
    painter->drawText((this->frameGeometry().width()-rmsg.width())/2, this->frameGeometry().height()-30, msg);
    painter->restore();

    // "<>"
    painter->save();
    painter->setFont(QFont("SimSun", 8));
    painter->setPen(QColor(213, 218, 220));
    QFontMetrics fwait(painter->font());
    QRect rwait = fwait.boundingRect(wait_);
    painter->drawText((this->frameGeometry().width()-rwait.width())/2, this->frameGeometry().height()-15, wait_);
    painter->restore();

    QSplashScreen::drawContents(painter);
}

void SplashScreen::init()
{
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &SplashScreen::wait_display);
    timer_->start(500);

    QString v = "1.0.x.x";
    QString date = "August 1, 2022";
    set_version(v);
    set_architecture();
    set_release_date(date);
}

void SplashScreen::set_version(QString &s)
{
    version_ = "version:" + s;
}

void SplashScreen::set_architecture()
{
    QString info =QSysInfo::currentCpuArchitecture() + " (" + QSysInfo::kernelType() + ")";
    architecture_ = info;
}

void SplashScreen::set_release_date(QString &s)
{
    release_date_ = s;
}

void SplashScreen::stop_timer()
{
    if(timer_->isActive())
        timer_->stop();
}

void SplashScreen::wait_display()
{
    wait_.insert(0,"<");
    wait_.append(">");
    if(wait_.size()>=20)
        wait_.clear();
    this->repaint();
}

void SplashScreen::mousePressEvent(QMouseEvent * event) {
    event->accept();
}

