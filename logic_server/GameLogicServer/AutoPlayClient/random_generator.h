#pragma once
#include "pre_header.h"

class random_generator {
private:
    static std::default_random_engine engine;

public:
    random_generator();
    ~random_generator();

    static int get_random_int(int min, int max);
};