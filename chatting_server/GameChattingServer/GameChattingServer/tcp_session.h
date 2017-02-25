#pragma once

#include "preheaders.h"

#include "chat_protocol.h"

enum user_status {
    lobby = 0,
    room = 1
};

enum post_type {
    verify_ans = 0,
    logout_ans = 1,
    chatting = 2
};

class tcp_server;

class tcp_session
{
private:
    boost::asio::ip::tcp::socket socket_;

    int session_id_;
    std::string user_key_;
    std::string user_id_;
    user_status status_;
    
    tcp_session* opponent_session_;
    tcp_server* server_;

    boost::array<BYTE, 1024> receive_buffer_;
    boost::container::deque<BYTE*> send_data_queue_;
    
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);

public:
    tcp_session(int session_id, boost::asio::io_service& io_service, tcp_server* server);
    ~tcp_session();

    boost::asio::ip::tcp::socket& get_socket() { return socket_; }

    int get_session_id() { return session_id_; }
    std::string get_user_key() { return user_key_; }
    std::string get_user_id() { return user_id_; }
    tcp_session* get_opponent_session() { return opponent_session_; }
    user_status get_status() { return status_; }
    
    bool set_user_key(std::string user_key);
    bool set_user_id(std::string user_id);
    bool set_opponent_session(tcp_session* opponent_session);
    bool set_status(user_status status);

    void post_send(const bool immediate, const int size, BYTE* data);
    void post_receive();
};