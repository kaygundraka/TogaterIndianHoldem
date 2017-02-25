#include "config.h"
#include "log_manager.h"
#include "redis_connector.h"


redis_connector::redis_connector()
{
}

redis_connector::~redis_connector()
{
}

bool redis_connector::init_singleton()
{
    std::string REDIS_IP;
    std::string REDIS_PORT;
    std::string REDIS_PW;

    if (!config::get_value("REDIS_IP", REDIS_IP))
    {
        LOG_ERROR << "redis_connector::init_singleton() - Cannot read a configuration file : REDIS_IP";
        return 0;
    }
    
    if (!config::get_value("REDIS_PORT", REDIS_PORT))
    {
        LOG_ERROR << "redis_connector::init_singleton() - Cannot read a configuration file : REDIS_PORT";
        return 0;
    }
    
    if (!config::get_value("REDIS_PW", REDIS_PW))
    {
        LOG_ERROR << "redis_connector::init_singleton() - Cannot read a configuration file : REDIS_PW";
        return 0;
    }


    conn_ = new redispp::Connection(REDIS_IP, REDIS_PORT, REDIS_PW, false);


    if (conn_ == nullptr)
    {
        LOG_ERROR << "init_singleton() : Redis connecting is failed.";
        return false;
    }
    
    LOG_INFO << "Redis connected.";
    return true;
}

bool redis_connector::release_singleton()
{
    delete conn_;
    
    if (conn_ != nullptr)
    {
        LOG_ERROR << "release_singleton() : Redis disconnecting is failed.";
        return false;
    }
       
    LOG_INFO << "Redis disconnected.";
    return true;
}

std::string redis_connector::get(std::string key)
{
    if (conn_->get(key).result().is_initialized())
        return conn_->get(key);

    return "";
}

bool redis_connector::del(std::string key)
{
    conn_->del(key);

    return true;
}

void redis_connector::set(std::string key, std::string value)
{
    conn_->set(key, value);
}