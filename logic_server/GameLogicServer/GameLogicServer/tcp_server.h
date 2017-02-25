#pragma once
#include "pre_headers.h"
#include "connected_session.h"
#include "critical_section.h"
#include <vector>

class tcp_server : public multi_thread_sync<tcp_server> {
private:
    tcp::acceptor acceptor_;
    std::vector<connected_session::pointer> connected_session_list_;
    
    int count;
    std::thread* keep_arrive_thread;
    bool end;

public:
    tcp_server(boost::asio::io_service& io_service, unsigned short port);
    void end_server();

private:
    void wait_accept();
    void handle_accept(connected_session::pointer new_connection, const boost::system::error_code& error);
    void check_connected_session();
};