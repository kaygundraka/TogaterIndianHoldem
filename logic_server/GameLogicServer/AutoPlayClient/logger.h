#pragma once
#include "pre_header.h"

class logger {
private:
    static bool debug_mode_;

public:
    static void is_debug_mode(bool mode)
    {
        debug_mode_ = mode;
    }

    static void print(std::string log)
    {
        if (debug_mode_)
            std::cout << log << std::endl;
    }
}; 