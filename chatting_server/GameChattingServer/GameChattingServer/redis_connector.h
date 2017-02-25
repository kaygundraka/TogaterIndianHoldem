#pragma once

#include "preheaders.h"

#include "singleton.h"


class redis_connector :public singleton<redis_connector>
{
private:
    redispp::Connection* conn_;
    
public:
    redis_connector();
    virtual ~redis_connector();

    virtual bool init_singleton();
    virtual bool release_singleton();

    std::string get(std::string key);
    bool del(std::string key);

    void set(std::string key, std::string value);
};