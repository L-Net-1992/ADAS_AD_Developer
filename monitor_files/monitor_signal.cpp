#include "monitor_files/monitor_signal.h"
#include "hdf5/hdf5files_handle.h"
#include <QWidget>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
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
    signal_.clear();
    dataset_.clear();
    checkbox_state_.clear();
}

void Replay::LoadFile()
{

}

int Replay::DataSetSize()
{
    return dataset_.size();
}

int Replay::SignalSize()
{
    return signal_.size();
}

QList<QPointF> Replay::GetSignalDataSet(const QString signal)
{
    QList<QPointF> tmp;
    auto it = dataset_.find(signal);
    if(it != dataset_.end()) {
        tmp = it.value();
    }
    return tmp;
}

QVector<QString> Replay::GetSignal()
{
    return signal_;
}

void Replay::DataSetClear()
{
    dataset_.clear();
}

void Replay::SendSignalData(QString name, QPointF data)
{
    emit SignalDataEvent(name, data);
}

void Replay::SetCheckboxState(QString signal, Qt::CheckState state)
{
    checkbox_state_[signal] = state;
}

Qt::CheckState Replay::GetCheckboxState(QString signal)
{
    return checkbox_state_.value(signal);
}

void Replay::AddSignalList(QString name, QColor color)
{
        signal_.push_back(name);
        emit SignalListEvent(name, color);
}

void Replay::SetSignalDataSet(QString signal, QList<QPointF> data)
{
    dataset_[signal] = data;
}

// reord
Record::Record(QObject *parent) : QObject(parent)
{
    start_time_ = 0;
    end_time_ = 0;
}

Record::~Record()
{

}

void Record::reset()
{
    start_time_ = 0;
    end_time_ = 0;
    file_name_ = "";
    data_.clear();

}

void Record::SetStartTime(unsigned long t)
{
    start_time_ = t;
}

unsigned long Record::GetStartTime()
{
    return start_time_;
}

void Record::SetEndTime(unsigned long t)
{
    end_time_ = t;
}

unsigned long Record::GetEndTime()
{
    return end_time_;
}

void Record::SetFileName(std::string name)
{
    file_name_ = name;
}

std::string Record::GetFileName()
{
    return file_name_;
}

void Record::SetSignalData(QString signal, QPointF data)
{
    data_[signal].push_back(data);
}

QVector<QPointF> Record::GetSignalData(const QString signal)
{
    QVector<QPointF> tmp;
    auto it = data_.find(signal);
    if(it != data_.end()) {
        tmp = it.value();
    }
    return tmp;
}

int Record::DataSize()
{
    return data_.size();
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

void Monitor::ClearSignalList()
{
    signal_name_.clear();
}

void Monitor::SendSignalData(QString name, QPointF data)
{
    emit SignalDataEvent(name, data);
    //    qDebug() << name << " " << data;
}

void Monitor::SendSignalDataGroup(QVector<QMap<QString, QPointF> > datas)
{
    emit SignalDataGroupEvent(datas);
}

QVector<QString> Monitor::GetSignalName()
{
    return signal_name_;
}

void Monitor::AddSignalCheckboxState(QString signal, Qt::CheckState state)
{
    signal_checkbox_state_[signal] = state;
}

void Monitor::ClearSignalCheckboxState()
{
    signal_checkbox_state_.clear();
}

Qt::CheckState Monitor::GetSignalCheckboxState(QString signal)
{
    return signal_checkbox_state_.value(signal);
}

QMap<QString, Qt::CheckState> Monitor::GetSignalCheckbox()
{
    return signal_checkbox_state_;
}

SignalTimer::SignalTimer(QString name, QVector<QPointF> data, QObject *parent)
    : QObject{parent}
{
    if(!data.size()) {
        qDebug() << name << " is empty";
        return;
    }
    GetSignalData(name, data);
//    tm_ = new QTimer(this);
//    tm_->setTimerType(Qt::PreciseTimer);
//    connect(tm_, &QTimer::timeout,this,&SignalTimer::Update);
//    tm_->setInterval(data_.at(0).x()*1000);
}

SignalTimer::~SignalTimer()
{
    qDebug() << "SignalTimer disconstruct";
}

void SignalTimer::GetSignalData(QString &name, QVector<QPointF> &data)
{
    name_ = name;
    data_ = data;
    size_ = 0;
}

void SignalTimer::Start(bool state)
{
    state_ = state;
    if(state_) {
        if(size_ < data_.size()) {
            QTimer::singleShot(data_.at(size_).x()*1000, this, &SignalTimer::SlotSingleShot);
        }
    }
    else {
        //
    }
}

void SignalTimer::Update()
{
    if(size_ < (data_.size()-1)) {
        tm_->stop();
        tm_->start((data_.at(size_+1).x()-data_.at(size_).x())*1000);
        emit Send(name_,data_.at(size_));
        size_++;
    }
    else {
        tm_->stop();
        emit Send(name_,data_.at(size_));
        emit Complate(data_.size());
    }
}

void SignalTimer::SlotSingleShot()
{
    emit Send(name_,data_.at(size_));
    size_++;
    if(state_ == 1) {
        if(size_ < data_.size()) {
            double interval = (data_.at(size_).x()-data_.at(size_-1).x()) * 1000;
            QTimer::singleShot(interval, this, &SignalTimer::SlotSingleShot);
        }
        else  {
            emit Complate(data_.size());
        }
    }
    //QTimer::singleShot(data_.at(0).x()*1000, this, &SignalTimer::SlotSingleShot);
}


}
