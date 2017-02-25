#pragma once
#include "pre_headers.h"
#include "singleton.h"
#include "critical_section.h"


typedef struct _DB_CONNECTION {
    MYSQL* connection_;
    MYSQL conn;
    MYSQL_RES* sql_result_;
    
    int query_state_;
    
    _DB_CONNECTION();
} DB_CONNECTION;

class query_enable {

};

class db_query {
public:
    query_enable* instance_;
    void (query_enable::*callback_func)(MYSQL_RES* sql_result_);
    
    std::string query_;

    void callback_event(MYSQL_RES* sql_result);

    db_query();
};

class database_connector : public singleton<database_connector>, public multi_thread_sync<database_connector> {
private:
    DB_CONNECTION session_;
    std::queue<db_query> queue_list_;

    bool is_work;
    std::thread* work_thread;

public:
    virtual bool init_singleton();
    virtual bool release_singleton();

    bool push_query(db_query query);
    void process_queue();
};

#define db_connector database_connector::get_instance()