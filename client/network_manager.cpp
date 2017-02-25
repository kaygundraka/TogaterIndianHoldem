#include "network_manager.h"
#include "logic_session.h"
#include "chat_session.h"
#include "channel_session.h"

network_manager::network_manager()
{
    session[0] = nullptr;
    session[1] = nullptr;
    session[2] = nullptr;
}

network_manager::~network_manager() {}

bool network_manager::init_singleton()
{
    thread_sync sync;

    if (session[0] == nullptr)
    {
        session[0] = new logic_session();
        session[0]->create();
    }

    if (session[1] == nullptr)
    {
        session[1] = new chat_session();
        session[1]->create();
    }

    if (session[2] == nullptr)
    {
        session[2] = new channel_session();
        session[2]->create();
    }

    for (int i = 0; i < 3; i++)
        if (!session[i]->create())
            return false;

    return true;
}

bool network_manager::release_singleton()
{
    thread_sync sync;

    for (int i = 0; i < 3; i++)
        if (session[i] != nullptr)
            if (!session[i]->destroy())
                return false;

    delete[] session;

    for (int i = 0; i < 3; i++)
        session[i] = nullptr;
   
    return true;
}

network_session* network_manager::get_session(SESSION_TYPE session_type)
{
    thread_sync sync;

    switch (session_type) {
    case SESSION_TYPE::LOGIC_SESSION:
        return session[0];
        break;

    case SESSION_TYPE::CHAT_SESSION:
        return session[1];
        break;

    case SESSION_TYPE::LOBBY_SESSION:
        return session[2];
        break;
    }

    return nullptr;
}

std::string network_manager::get_player_key()
{
    thread_sync sync;

    return player_key_;
}

std::string network_manager::get_player_id()
{
    thread_sync sync;

    return player_id_;
}

std::string network_manager::get_room_key()
{
    thread_sync sync;

    return room_key_;
}

void network_manager::set_player_key(std::string key)
{
    thread_sync sync;

    player_key_ = key;
}

void network_manager::set_player_id(std::string id)
{
    thread_sync sync;

    player_id_ = id;
}

void network_manager::set_room_key(std::string key)
{
    thread_sync sync;

    room_key_ = key;
}

void network_manager::set_player_history(channel_server::game_history history)
{
    thread_sync sync;

    player_history_ = history;
}

channel_server::game_history network_manager::get_player_history()
{
    thread_sync sync;
    
    return player_history_;
}