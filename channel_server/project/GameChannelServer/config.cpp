#include "config.h"

json_spirit::mValue config::json_;

bool config::init()
{
    if (json_.is_null())
    {
        std::ifstream config_file("config.json");
        if (!json_spirit::read(config_file, json_))
            return false;
    }

    return true;
}

bool config::get_value(std::string config_type, std::string key, int & value)
{
    if (!init())
        return false;
    json_spirit::mValue mvalue = json_.get_obj().at(config_type);
    json_spirit::mObject object = mvalue.get_obj();
    value = object.find(key)->second.get_int();

    return true;
}

bool config::get_value(std::string config_type, std::string key, std::string & value)
{
    if (!init())
        return false;

    json_spirit::mValue mvalue = json_.get_obj().at(config_type);
    json_spirit::mObject object = mvalue.get_obj();
    value = object.find(key)->second.get_str();

    return true;
}
