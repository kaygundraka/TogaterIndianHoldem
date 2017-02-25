#include "log_manager.h"



log_manager::log_manager()
{
    is = false;
    if (config::get_instance()->get_value("LOG_CONFIG", "MODE", log_mode))
    {
        if (!log_mode.compare("console"))
        {
            logger = spd::stdout_color_mt(log_mode.c_str());
            is = true;
        }
        else if (!log_mode.compare("basic_logger"))
        {
            logger = spd::basic_logger_mt(log_mode.c_str(), "logs/channel_server_log.txt");
            is = true;
        }
    }
}


log_manager::~log_manager()
{

}

std::string log_manager::get_log_mode()
{

}

void log_manager::set_log_mode()
{

}
