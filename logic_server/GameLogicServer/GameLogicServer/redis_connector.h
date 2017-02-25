#pragma once
#include "pre_headers.h"
#include "singleton.h"
#include "critical_section.h"

class redis_connector : public singleton<redis_connector>, public multi_thread_sync<redis_connector>{
private:
    redispp::Connection* conn;

public:
    virtual bool init_singleton();
    virtual bool release_singleton();

    redis_connector();
    virtual ~redis_connector();
    
    bool check_room(std::string room_key);
    bool remove_room_info(std::string room_key);
    bool remove_player_info(std::string player_key);
    std::string get_id(std::string player_key);
};