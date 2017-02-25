#pragma once

#include "preheaders.h"


#define LOG_DEBUG BOOST_LOG_SEV(file_log::get(), boost::log::trivial::debug)
#define LOG_INFO BOOST_LOG_SEV(file_log::get(), boost::log::trivial::info)
#define LOG_WARN BOOST_LOG_SEV(file_log::get(), boost::log::trivial::warning)
#define LOG_ERROR BOOST_LOG_SEV(file_log::get(), boost::log::trivial::error)

#define SYS_LOGFILE		"chatting_server_%Y%m%d.log"

//Narrow-char thread-safe logger.
typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;

//declares a global logger with a custom initialization
BOOST_LOG_GLOBAL_LOGGER(file_log, logger_t)