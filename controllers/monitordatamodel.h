#ifndef MONITORDATAMODEL_H
#define MONITORDATAMODEL_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QPointF>
#include <QColor>
#include <QDebug>

class MonitorDataModel : public QObject
{
    Q_OBJECT
public:
    explicit MonitorDataModel(QObject *parent = nullptr)
    {

    }

signals:
    void SignalListEvent(const QString name, QColor color);
    void SignalDataEvent(QString name, QPointF data);
private:
    // 信号清单
    std::vector<QString> signal_list_;
    // 信号数据集
    std::map<QString, QVector<QPointF>> signal_dataset_;
    std::map<QString, QPointF> signal_data;
    // 信号选择状态
    std::map<QString, Qt::CheckState> signal_checkbox_state_;


public:
    void clearModel()
    {
        signal_list_.clear();
        signal_checkbox_state_.clear();
        signal_data.clear();
        signal_dataset_.clear();
    }
    void addSignalList(QString name, QColor color)
    {
        signal_list_.push_back(name);
//        signal_data.at(name) = QPointF(0,0);
        emit SignalListEvent(name, color);
    }

    void removeSignalList(QString name)
    {
        Q_UNUSED(name);
    }
    std::vector<QString> get_signal_list()
    {
        return signal_list_;
    }
    // 数据集
    QPointF getSignalDataSet(const QString signal, size_t n)
    {
        QVector<QPointF> tmp;
        auto it = signal_dataset_.find(signal);
        if(it!=signal_dataset_.end()) {
            tmp = it->second;
        }
        return tmp.at(n);
    }

    void setSignalDataSet(QString signal, QVector<QPointF> data)
    {
        signal_dataset_[signal] = data;
    }

    size_t signalDataSetCount()
    {
        return signal_dataset_.size();
    }

    // 数据
    void addSignalData(QString name, QPointF data)
    {
//        qDebug() << "data: " << data;
//        signal_data.at(name) = data;
        emit SignalDataEvent(name, data);
    }
    void getSignalData()
    {

    }
    void setSignalData(std::map<QString,QPointF> &data)
    {
//        emit SignalDataEvent(data);
    }

    size_t signalCount() { return signal_list_.size();}
    size_t signalDataSize()
    {
        if(!signal_dataset_.empty())
            return signal_dataset_.begin()->second.size();
        return 0;
    }

    std::map<QString, bool> getSignalListState()
    {
        std::map<QString, bool> tmp;
        return tmp;
    }

    // 信号选择状态 std::map<QString, bool> signal_checkbox_state_
    void setSignalCheckboxState(QString signal, Qt::CheckState state)
    {
        signal_checkbox_state_[signal] = state;
    }
    Qt::CheckState getSignalCheckboxState(QString signal)
    {
        return signal_checkbox_state_.at(signal);
    }
    size_t getCheckBoxSize()
    {
        return signal_checkbox_state_.size();
    }

};

#endif // MONITORDATAMODEL_H
