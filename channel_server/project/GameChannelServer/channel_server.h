#pragma once
#include "server_session.h"
#include "friends_manager.h"
#include "match_manager.h"
#include "redis_connector.h"
#include "log_manager.h"
#include "config.h"

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service, friends_manager &friends, match_manager &match, packet_handler &packet_handler);
    ~tcp_server();

    void init();

    void start();
    
    void close_session(const int n_session_id);
    void process_packet(const int n_session_id, const char *p_data);
    void process_config(session * request_session, const char * packet, const int data_size);
    bool rematching_request(session *request_session);
    session* get_session(const int n_session_id) { return session_list_[n_session_id]; }

private:
    bool wait_accept();
    void handle_accept(session *p_session, const boost::system::error_code& error);

    void load_server_config();

    int seq_number_;
    boost::atomic<bool> accepting_flag_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::vector< session* > session_list_;
    std::deque<int> session_queue_;

    friends_manager &friends_manager_;
    match_manager &match_manager_;
    packet_handler &packet_handler_;

    boost::mutex session_queue_mtx;

    int max_thread, max_buffer_len, max_token_size;
    int max_session_count, port;

    boost::atomic<int> connections;
    boost::atomic<int> room_key_del;
    boost::atomic<int> token_del;
    boost::atomic<int> match_counts;
};

