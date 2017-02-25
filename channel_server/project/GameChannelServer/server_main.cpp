#include <iostream>

#include <boost/asio.hpp>        // boost lib 
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "log_manager.h"         // spd_logger
#include "protocol.h"            // protocol buffer
#include "redis_connector.h"     // redis util
#include "db_connector.h"        // mysql util
#include "channel_server.h"      // channel server

int main()
{
    boost::asio::io_service io_service;
    redis_connector redis_connector_main;                                                               // redis connector module 
    db_connector db_connector_main;                                                                     // mysql connector module
    packet_handler packet_handler_main;                                                                 // pakcet handle module
    friends_manager friends_manager_main(redis_connector_main, packet_handler_main, db_connector_main); // friends & join manage module
    match_manager match_manager_main(packet_handler_main, friends_manager_main, redis_connector_main);  // game mathing manage module
    tcp_server server(io_service,friends_manager_main, match_manager_main, packet_handler_main);        // main server
    int max_session_count;
    
    server.init();
    server.start();

    io_service.run();

    std::cout << "server terminated!!!!" << std::endl;
    getchar();
    return 0;
}
