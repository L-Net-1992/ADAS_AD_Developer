#ifndef MONITORSIGNAL_H
#define MONITORSIGNAL_H

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QTimerEvent>
#include <QDateTime>
#include <QDebug>
namespace monitor
{

class Replay : public QObject
{
    Q_OBJECT
public:
    explicit Replay(QObject *parent = nullptr);
    ~Replay();

    void Clear();
    void LoadFile();
    void AddSignalList(QString name, QColor color);
    void SetSignalDataSet(QString signal, QVector<QPointF> data);
    int  SignalDataSize();
    int  SignalListSize();
    QPointF GetSignalDataSet(const QString signal, size_t n);
    QVector<QString> GetSignalName();
    void SendSignalData(QString name, QPointF data);
    void SetSignalCheckboxState(QString signal, Qt::CheckState state);
    Qt::CheckState GetSignalCheckboxState(QString signal);

signals:
    void SignalListEvent(const QString name, QColor color);
    void SignalDataEvent(const QString name, QPointF data);
private:
    QVector<QString> signal_name_;
    QMap<QString, QVector<QPointF>> signal_dataset_;
    QMap<QString, Qt::CheckState> signal_checkbox_state_;
};

class Record : public QObject
{
    Q_OBJECT
public:
    explicit Record(QObject *parent = nullptr);
    ~Record();
    void reset();

signals:

private:
    unsigned long start_time_;
    unsigned long end_time_;
    std::string file_name_;
    QMap<QString, QVector<QPointF>> data_;
};

class Monitor : public QObject
{
    Q_OBJECT
public:
    explicit Monitor(QObject *parent = nullptr);
    ~Monitor();
    void Clear();
    void AddSignalList(QString name, QColor color);
    void SendSignalData(QString name, QPointF data);
    QVector<QString> GetSignalName();
    void SetSignalCheckboxState(QString signal, Qt::CheckState state);
    Qt::CheckState GetSignalCheckboxState(QString signal);

signals:
    void SignalListEvent(const QString name, QColor color);
    void SignalDataEvent(const QString name, QPointF data);

private:
    QVector<QString> signal_name_;
    QMap<QString,QPointF> signal_data_;
    QMap<QString,Qt::CheckState> signal_checkbox_state_;

    QMap<QString, QVector<QPointF>> signal_dataset_;
    QString file_;
    unsigned long start_time_;
    unsigned long end_time_;
};

class SignalTimer : public QObject
{
    Q_OBJECT
public:
    explicit SignalTimer(QObject *parent = nullptr)
        : QObject{parent}
    {
    }
    ~SignalTimer()
    {}
    int start(int ms)
    {
        return id_ = this->startTimer(ms, Qt::PreciseTimer);
    }
    void stop()
    {
        this->killTimer(id_);
    }
signals:

protected:
    void timerEvent(QTimerEvent *event) override
    {
        static int num=0;
        if(event->timerId() == id_)
        {
            qDebug() << QDateTime::currentMSecsSinceEpoch();
            stop();
            start(50+num*10);
            num++;
            if(num == 10)
            {
                this->killTimer(id_);
            }
        }
    }

private:
    int id_;
    int interval_;
};

}

#endif // MONITORSIGNAL_H
