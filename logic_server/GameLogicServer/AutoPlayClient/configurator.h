#pragma once
#include "pre_header.h"

class configurator {
private:
    static json_spirit::mValue json_;

    static bool init();

public:
    static bool get_value(std::string name, int& value);
    static bool get_value(std::string name, std::string& value);
};