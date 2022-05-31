#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QMouseEvent>
#include <QTimer>

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    SplashScreen(const QPixmap &pixmap);
    SplashScreen(const QPixmap &pixmap, const unsigned long ms);

protected:
    virtual void drawContents(QPainter *painter);
    virtual void mousePressEvent(QMouseEvent *event);

public:
    void init();
    void set_version(QString &s);
    void set_architecture();
    void set_release_date(QString &s);

public slots:
    void stop_timer();
    void wait_display(void);
private:
    QString version_;
    QString architecture_;
    QString release_date_;

    QString wait_;
    QTimer *timer_;
};

#endif // SPLASHSCREEN_H
