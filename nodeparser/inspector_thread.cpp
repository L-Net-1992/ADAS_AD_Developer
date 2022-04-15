//
// Created by liudian on 2022/3/19.
//

#include "inspector_thread.hpp"
#include <chrono>
#include <iostream>
using namespace boost;
using namespace std::chrono_literals;

void InspectorThread::run() {
    start_connect();
    for(;;) {
        io_.run_one_for(1s);
        if(isInterruptionRequested())
            break;
    }
}

InspectorThread::~InspectorThread() {
    requestInterruption();
    wait();

}

InspectorThread::InspectorThread(const QString &ip, QObject *parent): QThread(parent), remote_(asio::ip::address_v4::from_string(ip.toStdString()), 10081) {
}

void InspectorThread::start_connect() {
    socket_.async_connect(remote_, [this](const auto & error){
        connect_handler(error);
    });

}

void InspectorThread::connect_handler(const system::error_code &error) {
    if(!error) {
        start_read();
    } else {
        std::cerr << "socket connect: " << error.message() << std::endl;

    }

}

void InspectorThread::start_read() {
    asio::async_read(socket_, asio::buffer(&state_, sizeof(state)),
                     [this](const auto & error, auto bytes_transferred){ read_handler(error, bytes_transferred);});

}

void InspectorThread::read_handler(const system::error_code &error, std::size_t bytes_transferred) {
    if(!error) {
        emit varUpdated(state_.id.value(), state_.value.value());
        start_read();

    } else {

        std::cerr << "socket read: " << error.message() << std::endl;
    }

}
