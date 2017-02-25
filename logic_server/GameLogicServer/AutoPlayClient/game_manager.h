#pragma once
#include "network_manager.h"
#include "channel_session.h"
#include "chat_session.h"
#include "logic_session.h"
#include "singleton.h"
#include "my_thread_sync.h"

#define game_mgr game_manager::get_instance()

class game_manager : public singleton<game_manager>, public multi_thread_sync<game_manager>
{
public:
    bool send_friend_match_;
    bool accept_friend_match_;
    std::string friend_match_id_;

    bool end_game_;

    enum GAME_STATE { READY, ACCEPT, DENY, CONNECT, START, END };
    GAME_STATE state_;

    enum CHAT_TYPE { NORMAL, WHISPER, NOTICE, room};

    virtual bool init_singleton();
    virtual bool release_singleton();

    void play_game(std::string id, std::string password);
};