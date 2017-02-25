#include "log_manager.h"


namespace attrs = boost::log::attributes;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace logging = boost::log;


//Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(file_log, logger_t)
{
    logger_t lg;
    
    logging::add_common_attributes();

    boost::shared_ptr<sinks::synchronous_sink<sinks::text_file_backend>> sink = logging::add_file_log(
        boost::log::keywords::file_name = SYS_LOGFILE,
        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
        boost::log::keywords::format = (
            expr::stream << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
            << " [" << expr::attr< boost::log::trivial::severity_level >("Severity") << "]: "
            << expr::smessage
            )
    );

    // The sink will perform character code conversion as needed, according to the locale set with imbue()
    std::locale base_locale = boost::locale::generator()("en_US.UTF-8");
    sink->imbue(base_locale);
    
    
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );

    return lg;
}
