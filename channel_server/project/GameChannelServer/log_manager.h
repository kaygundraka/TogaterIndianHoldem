#pragma once
#include "config.h"

namespace spd = spdlog;

class log_manager:public singleton<log_manager>
{
public:
    log_manager();
    ~log_manager();
    std::string get_log_mode();
    void set_log_mode();
    inline std::shared_ptr<spd::logger> get_logger()
    {
        return logger;
    }
private:
    std::shared_ptr<spd::logger> logger;
    std::string log_mode;
    boost::atomic<bool> is;
};

