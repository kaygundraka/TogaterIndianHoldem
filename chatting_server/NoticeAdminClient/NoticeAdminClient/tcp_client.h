#pragma once

#include <iostream>

// -- boost --
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <boost/container/deque.hpp>

// -- protocol buffer --
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "chat_protocol.h"


class tcp_client
{
private:
    std::string user_id_;
    std::string key_;

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;

    boost::array<BYTE, 1024> receive_buffer_;
    boost::array<BYTE, 1024> send_buffer_;
    boost::container::deque<BYTE*> send_data_queue_;

    int packet_buffer_mark_;
    boost::array<BYTE, 2048> packet_buffer_;

    CRITICAL_SECTION lock_;

    bool is_login_;

    int TEMP_COUNT;

    
    void post_receive();

    void handle_connect(const boost::system::error_code& error);
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);
    
    bool process_packet(const int size);
    
public:
    tcp_client(boost::asio::io_service& io_service);
    ~tcp_client();
    
    bool is_connection() { return socket_.is_open(); }
    bool is_login() { return is_login_; }
    
    void connect(boost::asio::ip::tcp::endpoint endpoint);
    void close();

    void set_id(std::string user_id) { user_id_ = user_id; }
    void set_key(std::string key) { key_ = key; }


    void post_send(const bool immediate, const int size, BYTE* data);

    void post_verify_req();
    void post_enter_match(std::string opponent_id);
    void post_leave_match();

    void post_normal(std::string message);
    void post_whisper(std::string target_id, std::string message);
    void post_room(std::string message);
    void post_notice(std::string message);
};