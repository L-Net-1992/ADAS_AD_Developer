//
// Created by liudian on 2022/3/19.
//

#ifndef NODEDRIVING_INSPECTOR_HPP
#define NODEDRIVING_INSPECTOR_HPP
#include <QObject>
#include <boost/asio.hpp>
#include <QJsonDocument>
#include <QJsonObject>
#include <string>
#include <QString>
#include <QMap>
#include "inspector_thread.hpp"
class Inspector :public QObject {
    Q_OBJECT
private:
    boost::asio::io_context io_;
    std::string ip_;
    QJsonObject call(const QJsonObject & req);
    QMap<unsigned int, QString> varNameOfId_;
    InspectorThread *thread_;
    void start();
private Q_SLOTS:
    void onVarUpdated(unsigned int id, float value);
public:
    explicit Inspector(const QString & ip, QObject *parent = nullptr);
    void setParamValue(const QMap<QString, float> & value);
    QMap<QString, float> getParamValue();
    void setVarWatchState(const QMap<QString, bool> & state);
    QMap<QString, bool> getVarWatchState();
Q_SIGNALS:
    void varUpdated(const QString & varName, float value);


};


#endif //NODEDRIVING_INSPECTOR_HPP
