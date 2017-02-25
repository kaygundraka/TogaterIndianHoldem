#pragma once
#include "pre_header.h"
#include "my_thread_sync.h"

#define network_mgr network_manager::get_instance()

enum { BUFSIZE = 1024 };

class network_session : public multi_thread_sync<network_session> {
protected:
    tcp::socket* socket_;

    bool is_connected_;

    boost::array<BYTE, BUFSIZE> recv_buf_;
    boost::array<BYTE, BUFSIZE> send_buf_;

    boost::thread* work_thread_;
    boost::thread* recv_thread_;
    boost::asio::io_service io_service_;

public:
    network_session();
    ~network_session();

    bool is_run();

    void connect(std::string ip, std::string port);
    void disconnect();

    bool is_socket_open();

    virtual bool create() = 0;
    virtual bool destroy() = 0;

    virtual void handle_read() = 0;
};