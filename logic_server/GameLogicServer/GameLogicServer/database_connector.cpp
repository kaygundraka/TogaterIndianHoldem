#include "pre_headers.h"
#include "database_connector.h"
#include "configurator.h"
#include "log.h"

_DB_CONNECTION::_DB_CONNECTION()
{
    connection_ = nullptr;
    sql_result_ = nullptr;
    
    query_state_ = 0;
}

bool database_connector::init_singleton()
{
    thread_sync sync;

    mysql_init(&session_.conn);

    std::string ip, user, password, db_name;
    int port;
    configurator::get_value("mysql_server_ip", ip);
    configurator::get_value("mysql_server_port", port);
    configurator::get_value("mysql_user", user);
    configurator::get_value("mysql_password", password);
    configurator::get_value("mysql_db_name", db_name);

    session_.connection_ = mysql_real_connect(
        &session_.conn,
        ip.c_str(),
        user.c_str(),
        password.c_str(),
        db_name.c_str(),
        port,
        (char*)NULL, 0
    );

    if (session_.connection_ == nullptr)
        return false;

    work_thread = new std::thread(&database_connector::process_queue, this);

    if (work_thread == nullptr)
        return false;

    return true;
}

bool database_connector::release_singleton()
{
    thread_sync sync;

    mysql_close(session_.connection_);

    is_work = false;

    work_thread->join();

    return true;
}

bool database_connector::push_query(db_query query)
{
    thread_sync sync;

    queue_list_.push(query);

    return true;
}

void database_connector::process_queue()
{
    is_work = true;

    while (is_work)
    {
        thread_sync sync;

        if (queue_list_.empty())
            continue;

        auto query = queue_list_.front();

        session_.query_state_ = mysql_query(
            session_.connection_,
            query.query_.c_str()
        );

        if (session_.query_state_ != 0)
        {
            Log::WriteLog("error : query_stat_ != 0");
        }

        session_.sql_result_ = mysql_store_result(session_.connection_);

        /*if (session_.sql_result_ == nullptr)
        {
        }*/

        if (query.callback_func != nullptr)
            query.callback_event(session_.sql_result_);

        queue_list_.pop();
    }
}

db_query::db_query()
{
    callback_func = nullptr;
}

void db_query::callback_event(MYSQL_RES* sql_result)
{
    (instance_->*callback_func)(sql_result);

    if (sql_result != nullptr)
        mysql_free_result(sql_result);
}