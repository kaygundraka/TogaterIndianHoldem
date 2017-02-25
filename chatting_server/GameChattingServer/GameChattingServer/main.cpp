#include "preheaders.h"

#include "config.h"
#include "log_manager.h"
#include "redis_connector.h"

#include "tcp_server.h"


int main()
{
    LOG_INFO << "main() start";

    redis_connector::get_instance()->init_singleton();
    
    //for (int n = 0; n < 1000; n++)
    //    redis_connector::get_instance()->set("user" + std::to_string(n), "user" + std::to_string(n));

    //return 0;

    boost::asio::io_service io_service;
    
    int SERVER_PORT;
    int MASTER_BUFFER_LEN;
    int MAX_SESSION_COUNT;

    if (!config::get_value("SERVER_PORT", SERVER_PORT))
    {
        LOG_ERROR << "main() - Cannot read a configuration file : SERVER_PORT";
        return 0;
    }
    
    if (!config::get_value("MASTER_BUFFER_LEN", MASTER_BUFFER_LEN))
    {
        LOG_ERROR << "main() - Cannot read a configuration file : MASTER_BUFFER_LEN";
        return 0;
    }
    
    if (!config::get_value("MAX_SESSION_COUNT", MAX_SESSION_COUNT))
    {
        LOG_ERROR << "main() - Cannot read a configuration file : SERVER_PORT";
        return 0;
    }


    tcp_server server(io_service, SERVER_PORT, MASTER_BUFFER_LEN);


    if (!server.init(MAX_SESSION_COUNT))
        return 0;
    
    if (!server.start())
        return 0;

    
    boost::thread_group threads;
    
    // There are (7 + 1) threads.
    for (int i = 0; i < 7; i ++)
        threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    io_service.run();
    

    threads.join_all();
    
    std::cout << "Server Closed" << std::endl;

    return 0;
}