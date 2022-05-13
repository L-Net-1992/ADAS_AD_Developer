#include "splashscreen.h"
#include <QPainter>
#include <QSysInfo>
#include <QThread>
#include <QDebug>
SplashScreen::SplashScreen(const QPixmap &pixmap) : QSplashScreen(pixmap)
{
    init();
    this->showMessage(QStringLiteral("正在加载资源文件，请稍后……"), Qt::AlignHCenter|Qt::AlignBottom, Qt::white);

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

    QString copyright;
    copyright = "Copyright © 2022 The Aicc Company, All Rights Reserved.";
    painter->setFont(QFont("SimSun", 8));
    painter->setPen(QColor(213, 218, 220));
    QFontMetrics fm(painter->font());
    QRect rec = fm.boundingRect(copyright);
    painter->drawText((this->frameGeometry().width()-rec.width())/2, this->frameGeometry().height()-120, copyright);

    QSplashScreen::drawContents(painter);
}

void SplashScreen::init()
{
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

void SplashScreen::mousePressEvent(QMouseEvent * event) {
    event->accept();
}

