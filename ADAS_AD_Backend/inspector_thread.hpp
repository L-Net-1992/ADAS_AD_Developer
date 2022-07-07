//
// Created by liudian on 2022/3/19.
//

#ifndef NODEDRIVING_INSPECTOR_THREAD_HPP
#define NODEDRIVING_INSPECTOR_THREAD_HPP
#include <QThread>
#include <QString>
#include <boost/asio.hpp>
#include <boost/endian.hpp>

class InspectorThread: public QThread {
    Q_OBJECT
private:
    struct state {
        boost::endian::big_uint32_buf_t id;
        boost::endian::big_float32_buf_t value;
    };
private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::socket socket_{io_};
    state state_{};
    boost::asio::ip::tcp::endpoint remote_;
    void start_connect();
    void connect_handler(const boost::system::error_code& error);
    void start_read();
    void read_handler(const boost::system::error_code& error, std::size_t bytes_transferred);
protected:
    void run() override;

public:
    explicit InspectorThread(const QString & ip, QObject * parent = nullptr);
    ~InspectorThread() override;
Q_SIGNALS:
    void varUpdated(unsigned int id, float value);

};


#endif //NODEDRIVING_INSPECTOR_THREAD_HPP
