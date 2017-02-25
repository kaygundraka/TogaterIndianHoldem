#include "pre_headers.h"
#include "tcp_server.h"
#include "log.h"
#include "redis_connector.h"
#include "logic_worker.h"
#include "configurator.h"
#include "database_connector.h"

boost::thread_group io_thread;
boost::asio::io_service service;
tcp_server* server = nullptr;

BOOL WINAPI ConsolHandler(DWORD handle)
{
    switch (handle)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT: 
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
        service.stop();

        io_thread.join_all();

        server->end_server();

        if (database_connector::get_instance()->release_singleton())
            Log::WriteLog(_T("release_database_manager"));

        if (logic_worker::get_instance()->release_singleton())
            Log::WriteLog(_T("release_logic_manager"));

        if (redis_connector::get_instance()->release_singleton())
            Log::WriteLog(_T("release_redis_manager"));

        Log::WriteLog(_T("server_close"));
        //log_mgr->release_singleton();

        if (server != nullptr)
        {
            server->end_server();
            delete server;
        }

        return false;
    }

    return false;
}

std::string _itoa(int i)
{
    char temp[10] = "";
    itoa(i, temp, 10);
    
    return temp;
}

int main(int argc, char* argv[])
{
    SetConsoleCtrlHandler(ConsolHandler, true);

    Log::WriteLog(_T("start logic server"));

    try
    {
        if (!redis_connector::get_instance()->init_singleton())
        {
            Log::WriteLog(_T("failed_init_redis_manager"));

            throw;
        }
        else
            Log::WriteLog(_T("init_redis_manager"));

        if (!logic_worker::get_instance()->init_singleton())
        {
            Log::WriteLog(_T("failed_init_logic_worker"));

            throw;
        }
        else
            Log::WriteLog(_T("init_logic_manager"));

        if (!database_connector::get_instance()->init_singleton())
        {
            Log::WriteLog(_T("failed_init_database_manager"));
            throw;
        }
        else
            Log::WriteLog(_T("init_database_manager"));
        
        int port;

        if (!configurator::get_value("port", port))
        {
            Log::WriteLog(_T("configurator_error, get_value:%d"), port);
            throw;
        }

        server = new tcp_server(service, port);

        for (int i = 0; i < 7; i++)
            io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &service));
            
        //getchar();
        //service.stop();

        io_thread.join_all();
    }
    catch (std::exception& e)
    {
        Log::WriteLog(_T("%s"), e.what());
    }

    server->end_server();

    if (database_connector::get_instance()->release_singleton())
        Log::WriteLog(_T("release_database_manager"));

    if (logic_worker::get_instance()->release_singleton())
        Log::WriteLog(_T("release_logic_manager"));

    if (redis_connector::get_instance()->release_singleton())
        Log::WriteLog(_T("release_redis_manager"));

    if (server != nullptr)
    {
        server->end_server();
        delete server;
    }

    Log::WriteLog(_T("server_close"));

    //log_mgr->release_singleton();
        
    return 0;
}