#include "db_connector.h"


db_connector::db_connector()
{
    load_mysql_config();
    init_instance = new MYSQL[pool_size];
    this->init();
}

db_connector::~db_connector()
{
    for (int i = 0; i < pool_size; i++)
    {
        mysql_close(connection_pool[i]);
    }
}

void db_connector::init()
{
    for (int i = 0; i < pool_size; i++)
    {
        mysql_init(&init_instance[i]);
        MYSQL *new_connection = mysql_real_connect(&init_instance[i], ip.c_str(), id.c_str(), pwd.c_str(), db_name.c_str(), port, (char *)nullptr, i);
        if (new_connection == nullptr)
        {
            std::cout << "connection error" << std::endl;
        }
        else
        {
            connection_pool.push_back(new_connection);
            connection_que.push_back(i);
        }
    }
}

bool db_connector::get_query_user_info(const std::string request_id, int & win, int & lose, int & rating)
{
    char query[100];
    sprintf(query, "SELECT win,lose,rating FROM user_info WHERE id='%s'", request_id.c_str());
    
    int index = 0;
    MYSQL *mysql_connection;
    while((mysql_connection = get_connection(index)) == nullptr);

    int query_state = mysql_query(mysql_connection, query);
    if (query_state != 0)
    {
        release_connection(index);
        return false;
    }

    MYSQL_RES *query_result = mysql_store_result(mysql_connection);
    MYSQL_ROW sql_row;
    if (query_result->row_count <= 0)
    {
        release_connection(index);
        return false;
    }

    while ((sql_row = mysql_fetch_row(query_result)) != NULL)
    {
        win = atoi(sql_row[0]);
        lose = atoi(sql_row[1]);
        rating = atoi(sql_row[2]);
    }
    
    if (query_result->lengths > 0)
    {
        mysql_free_result(query_result);
    }
    release_connection(index);
    return true;
}

bool db_connector::get_user_friends_list(const std::string request_id, std::vector<std::string>& friends_list)
{
    char query[100];
    sprintf(query, "SELECT id_2 FROM friend_info WHERE id_1='%s'", request_id.c_str());
    
    int index = 0;
    MYSQL *mysql_connection;
    while ((mysql_connection = get_connection(index)) == nullptr);
    int query_state = mysql_query(mysql_connection, query);
    if (query_state != 0)
    {
        std::cout << "[" << query << "]" << " : ERROR" << endl;
        release_connection(index);
        return false;
    }
    MYSQL_RES *query_result = mysql_store_result(mysql_connection);
    MYSQL_ROW sql_row;
    while ((sql_row = mysql_fetch_row(query_result)) != NULL)
    {
        friends_list.push_back(sql_row[0]);
    }

    if (query_result->lengths > 0)
    {
        mysql_free_result(query_result);
    }
    release_connection(index);
    return true;
}

bool db_connector::add_user_frineds_list(const std::string request_id, const std::string target_id)
{
    int index = 0;
    MYSQL *mysql_connection;
    while ((mysql_connection = get_connection(index)) == nullptr);

    if (!check_repetition(mysql_connection, request_id, target_id))
    {
        release_connection(index);
        return false;
    }
    char query[100] = { 0, };
    sprintf(query, "INSERT INTO friend_info (id_1,id_2) VALUES ('%s','%s')", request_id.c_str(), target_id.c_str());
    
    int query_state = mysql_query(mysql_connection, query);
    release_connection(index);
    if (query_state != 0)
    {
        return false;
    }
    return true;
}

bool db_connector::del_user_frineds_list(const std::string request_id, const std::string target_id)
{
    int index = 0;
    MYSQL *mysql_connection;
    while ((mysql_connection = get_connection(index)) == nullptr);
    
    char query[100] = { 0, };
    sprintf(query, "DELETE FROM friend_info WHERE id_1='%s' AND id_2='%s'", request_id.c_str(),target_id.c_str());

    int query_state = mysql_query(mysql_connection, query);
    int row = mysql_affected_rows(mysql_connection);
    release_connection(index);
    if (query_state != 0 || row == 0)
    {
        return false;
    }
    return true;
}

bool db_connector::reconncet(const int index)
{
    return false;
}

bool db_connector::check_repetition(MYSQL *con, const std::string request_id, const std::string target_id)
{
    char query[100] = { 0, };
    sprintf(query, "SELECT * FROM friend_info WHERE id_1 = '%s' and id_2 = '%s'", request_id.c_str(), target_id.c_str());
    int query_state = mysql_query(con, query);
    if (query_state == 0)
    {
        MYSQL_RES *result = mysql_store_result(con);
        MYSQL_ROW sql_row = mysql_fetch_row(result);
        if (sql_row == nullptr)
        {
            mysql_free_result(result);
            return true;
        }
        mysql_free_result(result);
    }
    return false;
}

MYSQL * db_connector::get_connection(int & index)
{
    connection_mtx.lock();

    if (connection_que.empty())
    {
        index = -1;
        connection_mtx.unlock();
        return nullptr;
    }
    index = connection_que.front();
    connection_que.pop_front();

    connection_mtx.unlock();

    return connection_pool[index];
}

void db_connector::release_connection(const int index)
{
    connection_mtx.lock();
    connection_que.push_back(index);
    connection_mtx.unlock();
}

void db_connector::load_mysql_config()
{
    config::get_instance()->get_value("MYSQL_CONFIG", "IP", ip);
    config::get_instance()->get_value("MYSQL_CONFIG", "PORT", port);
    config::get_instance()->get_value("MYSQL_CONFIG", "ID", id);
    config::get_instance()->get_value("MYSQL_CONFIG", "PWD", pwd);
    config::get_instance()->get_value("MYSQL_CONFIG", "DB_NAME", db_name);
    config::get_instance()->get_value("MYSQL_CONFIG", "POOL_SIZE", pool_size);
}