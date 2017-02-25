#pragma once
#include "config.h"
#include "log_manager.h"


class tcp_server;

class user
{
public:
    inline void set_user_info(const int rating, const int battle_history, const int win, const int lose, std::string user_id)
    {
        rating_score_ = rating;
        battle_history_ = battle_history;
        win_ = win;
        lose_ = lose;
        user_id_ = user_id;
    }
        
    inline int get_rating() { return rating_score_; }
    inline int get_battle_history() { return battle_history_; }
    inline int get_win() { return win_; }
    inline int get_lose() { return lose_; }
    inline std::string get_user_id() { return user_id_; }
    inline void set_friends_count(int count) { friends_count_ = count; }
    inline int get_friends_count() { return friends_count_; }

private:
    int rating_score_, battle_history_ ,win_ ,lose_, friends_count_;
    std::string user_id_;
};

class session:public user
{
public:
    session(int session_id, boost::asio::io_service &io_service, tcp_server* p_channel_serv, const int token_size, const int max_buffer_len);
    ~session();
    
    inline int get_session_id() { return session_id_; }
    inline boost::asio::ip::tcp::socket& get_socket() { return socket_; }
    
    
    void init();
    void wait_receive();
    void wait_send(const bool immediately, const int send_data_size, char *send_data);

    inline void set_token(std::string token) { token_ = token; }
    inline std::string get_token() { return token_; }

    inline void set_room_key(std::string room_key) { room_key_ = room_key; }
    inline std::string get_room_key() { return room_key_; }
    inline void set_status(status state) { stat_ = state; }
    inline status get_status() { return stat_; } 
    
    void control_timer_rematch(int sec, bool is_set);
    void rematch(const boost::system::error_code & error);
private:
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);

    std::string token_;
    std::string room_key_;

    int token_size_, max_buffer_len_;
    boost::atomic<status> stat_;
    int session_id_;
    boost::asio::ip::tcp::socket socket_;

    boost::asio::steady_timer match_timer_;
 
    char *receive_buffer_,*temp_buffer_;
    int packet_buffer_mark_;
    char *packet_buffer_;

    std::deque<char *> send_data_queue_;
    tcp_server *channel_serv_;
};
