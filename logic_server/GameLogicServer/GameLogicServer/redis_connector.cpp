#include "pre_headers.h"
#include "redis_connector.h"
#include "log.h"
#include "configurator.h"

redis_connector::redis_connector() {}
redis_connector::~redis_connector() {}

bool redis_connector::init_singleton()
{
    thread_sync sync;

    std::string ip;
    configurator::get_value("redis_server_ip", ip);

    std::string port;
    configurator::get_value("redis_server_port", port);

    Log::WriteLog(_T("redis_connector:connect, ip: %s, port: %s"), ip.c_str(), port.c_str());
    conn = new redispp::Connection(ip, port, "password", false);

    if (conn == nullptr)
    {
        Log::WriteLog(_T("redis_connector:conn_is_null"));
        return false;
    }

    return true;
}

bool redis_connector::release_singleton() 
{ 
    return true; 
}

bool redis_connector::check_room(std::string room_key)
{
    thread_sync sync;

    auto value = conn->get(room_key);

    if (!value.result().is_initialized())
    {
        Log::WriteLog(_T("redis_connector:check_room(%s), room_key_is_null"), room_key.c_str());
        return false;
    }
    
    int i = atoi(value.result().get().c_str());
    i++;

    char value_str[11] = "";

    itoa(i, value_str, 10);

    conn->set(room_key, value_str);

    return true;
}

bool redis_connector::remove_room_info(std::string room_key)
{
    thread_sync sync;

    if (!conn->del(room_key).result())
    {
        Log::WriteLog(_T("redis_connector:remove_room_info(%s), room_key_is_null"), room_key.c_str());
        return false;
    }

    return true;
}

bool redis_connector::remove_player_info(std::string player_key)
{
    thread_sync sync;

    if (!conn->del(player_key).result())
    {
        Log::WriteLog(_T("redis_connector:remove_player_info(%s), player_key_is_null"), player_key.c_str());
        return false;
    }

    return true;
}

std::string redis_connector::get_id(std::string player_key)
{
    thread_sync sync;

    auto value = conn->get(player_key);

    if (!value.result().is_initialized())
    {
        Log::WriteLog(_T("redis_connector:get_id(%s), player_id is null"), player_key.c_str());
        return "";
    }

    return value.result().get().c_str();
}