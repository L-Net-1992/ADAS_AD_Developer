#include "monitor_files/monitor_signal.h"
#include "hdf5/hdf5files_handle.h"
#include <QWidget>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QDebug>

using namespace utility;
namespace monitor {

// reaply
Replay::Replay(QObject *parent)
    : QObject{parent}
{

}

Replay::~Replay()
{

}

void Replay::Clear()
{
    signal_name_.clear();
    signal_dataset_.clear();
}

void Replay::LoadFile()
{

}

int Replay::SignalDataSize()
{
    if(signal_dataset_.size()) {
        return signal_dataset_.begin().value().size();
    }
    else {
        return 0;
    }
}

int Replay::SignalListSize()
{
    return signal_name_.size();
}

QPointF Replay::GetSignalDataSet(const QString signal, size_t n)
{
    QVector<QPointF> tmp;
    auto it = signal_dataset_.find(signal);
    if(it!=signal_dataset_.end()) {
        tmp = it.value();
    }
    return tmp.at(n);
}

QVector<QString> Replay::GetSignalName()
{
    return signal_name_;
}

void Replay::SendSignalData(QString name, QPointF data)
{
    emit SignalDataEvent(name, data);
}

void Replay::SetSignalCheckboxState(QString signal, Qt::CheckState state)
{
    signal_checkbox_state_[signal] = state;
}

Qt::CheckState Replay::GetSignalCheckboxState(QString signal)
{
    return signal_checkbox_state_.value(signal);
}

void Replay::AddSignalList(QString name, QColor color)
{
        signal_name_.push_back(name);
        emit SignalListEvent(name, color);
}

void Replay::SetSignalDataSet(QString signal, QVector<QPointF> data)
{
    signal_dataset_[signal] = data;
}

// reord
Record::Record(QObject *parent) : QObject(parent)
{
    start_time_ = 0;
    end_time_ = 0;
    qDebug() << "record";
}

Record::~Record()
{
    qDebug() << "~record";
}

void Record::reset()
{
    start_time_ = 0;
    end_time_ = 0;
    file_name_ = "";
    data_.clear();

}

// wave
Monitor::Monitor(QObject *parent) : QObject(parent)
{

}

Monitor::~Monitor()
{

}

void Monitor::Clear()
{
    signal_name_.clear();
    signal_data_.clear();
}

void Monitor::AddSignalList(QString name, QColor color)
{
    signal_name_.push_back(name);
    emit SignalListEvent(name, color);
}

void Monitor::SendSignalData(QString name, QPointF data)
{
    emit SignalDataEvent(name, data);
//    qDebug() << name << " " << data;
}

QVector<QString> Monitor::GetSignalName()
{
    return signal_name_;
}

void Monitor::SetSignalCheckboxState(QString signal, Qt::CheckState state)
{
    signal_checkbox_state_[signal] = state;
}

Qt::CheckState Monitor::GetSignalCheckboxState(QString signal)
{
    return signal_checkbox_state_.value(signal);
}


}
