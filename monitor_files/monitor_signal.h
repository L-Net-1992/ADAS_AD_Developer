#ifndef MONITORSIGNAL_H
#define MONITORSIGNAL_H

#include <QWidget>
#include <QVector>
#include <QMap>

namespace monitor
{

class Replay : public QWidget
{
    Q_OBJECT
public:
    explicit Replay(QWidget *parent = nullptr);
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

signals:

private:

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

}

#endif // MONITORSIGNAL_H
