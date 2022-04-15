//
// Created by liudian on 2022/3/19.
//

#include "inspector.hpp"
#include <sstream>
#include <array>
#include <QDebug>
#include <iostream>

using namespace boost;

Inspector::Inspector(const QString &ip, QObject *parent) : QObject(parent), ip_(ip.toStdString()),
                                                           thread_(new InspectorThread(ip, this)) {
    connect(thread_, &InspectorThread::varUpdated, this, &Inspector::onVarUpdated);
    start();
}

QJsonObject Inspector::call(const QJsonObject &req) {

    asio::ip::tcp::socket s{io_};
    s.connect(asio::ip::tcp::endpoint{asio::ip::address_v4::from_string(ip_), 10080});
    std::string req_json = QJsonDocument{req}.toJson().toStdString();
    asio::write(s, asio::buffer(req_json));
    s.shutdown(boost::asio::socket_base::shutdown_send);
    std::ostringstream response_json;
    for (;;) {
        std::array<char, 4096> buf{};
        boost::system::error_code error;
        size_t size = s.read_some(boost::asio::buffer(buf), error);

        if (error == boost::asio::error::eof)
            break;
        else if (error)
            throw boost::system::system_error(error);

        response_json << std::string{buf.data(), size};
    }
    std::string response = response_json.str();
    return QJsonDocument::fromJson(QByteArray(response.data(), response.size())).object();
}

void Inspector::start() {
    QJsonObject req;
    req["action"] = "get_var_id";
    QJsonObject response = call(req);
    for (const auto &k: response.keys()) {
        varNameOfId_[response[k].toInt()] = k;
    }
    thread_->start();

}

void Inspector::setParamValue(const QMap<QString, float> &value) {
    QJsonObject req;
    req["action"] = "set_param_value";
    QJsonObject data;
    const auto &list = value.keys();
    for (const auto &k: list) {
        data[k] = value[k];
    }
    req["data"] = data;
    call(req);

}

QMap<QString, float> Inspector::getParamValue() {
    QMap<QString, float> map{};
    QJsonObject req;
    req["action"] = "get_param_value";
    QJsonObject response = call(req);
    for (const auto &k: response.keys()) {
        map[k] = static_cast<float>(response[k].toDouble());
    }
    return map;
}

void Inspector::setVarWatchState(const QMap<QString, bool> &state) {
    QJsonObject req;
    req["action"] = "set_var_watch_state";
    QJsonObject data;
    const auto &list = state.keys();
    for (const auto &k: list) {
        data[k] = state[k];
    }
    req["data"] = data;
    call(req);

}

QMap<QString, bool> Inspector::getVarWatchState() {
    QMap<QString, bool> map{};
    QJsonObject req;
    req["action"] = "get_var_watch_state";
    QJsonObject response = call(req);
    for (const auto &k: response.keys()) {
        map[k] = response[k].toBool();
    }
    return map;
}

void Inspector::onVarUpdated(unsigned int id, float value) {
    auto iter = varNameOfId_.find(id);
    if(iter != varNameOfId_.end()) {
        emit varUpdated(iter.value(), value);
    } else {
        std::cerr << "warning: var id " <<  id << " name not exists" << std::endl;
    }

}
