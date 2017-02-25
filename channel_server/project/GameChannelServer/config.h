#pragma once
#include "protocol.h"


class config:public singleton<config>
{
public:
    
private:
    bool init();
    static json_spirit::mValue json_;
public:
    bool get_value(std::string config_type, std::string key, int& value);
    bool get_value(std::string config_type, std::string key, std::string& value);
};