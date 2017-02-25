#pragma once

#include "tcp_session.h"

enum reserved_type {
    VERIFY_TRUE = 0,
    VERIFY_FALSE,
    LOGOUT_TRUE,
    LOGOUT_FALSE,
    WHISPER_ERROR
};


class tcp_server
{
private:
    bool is_accepting_;

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;

    boost::container::deque<int> session_queue_;
    boost::container::vector<tcp_session*> session_list_;
    boost::unordered_map<std::string, tcp_session*> connected_session_map_;

    boost::circular_buffer<boost::array<BYTE, 1024>> master_data_queue_;

    bool make_reserved_message();
    boost::array<boost::array<BYTE, 1024>, 5> reserved_message;
    boost::array<int, 5> reserved_message_size;
    

    void post_accept();
    void handle_accept(tcp_session* session, const boost::system::error_code& error);

public:
    tcp_server(boost::asio::io_service& io_service, int server_port, int master_buffer_len);
    ~tcp_server();

    bool init(const int max_session_count);
    bool start();


    boost::asio::io_service& get_io_service() { return io_service_; }
    boost::circular_buffer<boost::array<BYTE, 1024>> get_master_data_queue() { return master_data_queue_; }

    void close_session(const int session_id);

    void process_packet(const int session_id, const int size, BYTE* packet);

    boost::asio::strand strand_accept_;
    boost::asio::strand strand_close_;
    boost::asio::strand strand_receive_;
    boost::asio::strand strand_send_;
};