#pragma once
#include "server_session.h"
#include "log_manager.h"
#include "redis_connector.h"
#include "db_connector.h"
#include "config.h"

class friends_manager
{
public:
    friends_manager(redis_connector& redis_connector, packet_handler& handler, db_connector &mysql);
    ~friends_manager();
    bool lobby_login_process(session *login_session, const char *packet, const int packet_size);
    bool del_redis_token(std::string token);
    bool lobby_logout_process(session *logout_session, const char *packet, const int packet_size);
    bool search_user(session * request_session, std::string target_id);
    bool add_friends(session *request_session, std::string target_id);
    bool del_friends(session *request_session, std::string target_id);
    void process_friends_function(session *request_session, const char *packet, const int packet_size);
    session * find_id_in_user_map(std::string target_id);
    bool del_id_in_user_map(std::string target_id);
    bool add_id_in_user_map(session *request_session, std::string request_id);
private:
    boost::atomic<int> del_count;
    redis_connector& redis_connector_;
    packet_handler& packet_handler_;
    std::unordered_map<std::string, session *> user_id_map_;

    boost::mutex user_id_map_mtx;
    db_connector &db_connector_;
};