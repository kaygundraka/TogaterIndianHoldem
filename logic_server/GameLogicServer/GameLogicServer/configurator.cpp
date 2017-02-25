#include "pre_headers.h"
#include "configurator.h"

json_spirit::mValue configurator::json_;

bool configurator::init()
{
    if (json_.is_null())
    {
        std::ifstream is("config.json");

        if (!json_spirit::read(is, json_))
            return false;
    }

    return true;
}

bool configurator::get_value(std::string name, int& value)
{
    if (!init())
        return false;

    json_spirit::mObject object = json_.get_obj();

    value = object.find(name)->second.get_int();

    return true;
}

bool configurator::get_value(std::string name, std::string& value)
{
    if (!init())
        return false;

    json_spirit::mObject object = json_.get_obj();

    value = object.find(name)->second.get_str();

    return true;
}