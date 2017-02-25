#include "friends_manager.h"




friends_manager::friends_manager(redis_connector& redis_connector, packet_handler & handler, db_connector &mysql)
    : redis_connector_(redis_connector)
    , packet_handler_(handler)
    , db_connector_(mysql)
    , del_count(0)
{
}

friends_manager::~friends_manager()
{
    user_id_map_.clear();
}

bool friends_manager::lobby_login_process(session *request_session, const char * packet, const int packet_size)
{
    if (request_session->get_socket().is_open())
    {
        join_request message;
        packet_handler_.decode_message(message, packet, packet_size);
        std::string token = message.token();
        std::string id;
        join_response response_message;
        if ((redis_connector_.get_redis_kv(token, id)) && (find_id_in_user_map(id) == nullptr))
        {
            response_message.set_success(true);
            game_history *history = response_message.mutable_history();

            int rating = 0;
            int total_games = 0;
            int win = 0;
            int lose = 0;
            int friends_count = 0;
            std::vector<std::string> friends_list;

            if (db_connector_.get_query_user_info(id, win, lose, rating))
            {
                total_games = win + lose;
            }
            else
            {   
                redis_connector_.del_redis_key(token);
                response_message.set_success(false);
                request_session->wait_send(false, response_message.ByteSize() + packet_header_size, packet_handler_.incode_message(response_message));
                return false;
            }
            
            if (db_connector_.get_user_friends_list(id, friends_list))
            {
                friends_count = friends_list.size();
                request_session->set_friends_count(friends_count);
            }
            else
            {
                response_message.set_success(false);
                request_session->wait_send(false, response_message.ByteSize() + packet_header_size, packet_handler_.incode_message(response_message));
                return false;
            }

            request_session->set_user_info(rating, total_games, win, lose, id);
            request_session->set_token(token);

            for (int i = 0; i < friends_count; i++)
            {
               basic_info *friends_id = response_message.add_friends_list();
               friends_id->set_id(friends_list[i]);
            }

            add_id_in_user_map(request_session, request_session->get_user_id());
        
            history->set_total_games(total_games);
            history->set_rating_score(rating);
            history->set_win(win);
            history->set_lose(lose);

            request_session->set_status(status::LOGIN);
            request_session->wait_send(false, response_message.ByteSize() + packet_header_size, packet_handler_.incode_message(response_message));
            log_manager::get_instance()->get_logger()->info("[Join Success] -User_id [{0:s}] -Session_id [{1:d}]", request_session->get_user_id(), request_session->get_session_id());
            return true;
        }
        else
        {
            log_manager::get_instance()->get_logger()->warn("[Join Fail] -Session_id [{0:d}]", request_session->get_session_id());
            request_session->set_status(status::LOGOUT);
            response_message.set_success(false);
            request_session->wait_send(false, response_message.ByteSize() + packet_header_size, packet_handler_.incode_message(response_message));
            return false;
        }
    }
    return false;
}

bool friends_manager::del_redis_token(std::string token)
{
    bool ret = redis_connector_.del_redis_key(token);
    return ret;
}

bool friends_manager::lobby_logout_process(session *request_session, const char *packet, const int packet_size)
{
    std::string id; 
    
    if (redis_connector_.get_redis_kv(request_session->get_token(), id))
    {
        logout_response response_message;
        redis_connector_.del_redis_key(request_session->get_token());
        del_id_in_user_map(request_session->get_user_id());

        request_session->set_status(status::LOGOUT);
        request_session->wait_send(false, response_message.ByteSize() + packet_header_size, packet_handler_.incode_message(response_message));
        
        log_manager::get_instance()->get_logger()->info("[Logout Success] -User_id [{0:s}]", request_session->get_user_id());
        return true;
    }
    else
    {
        log_manager::get_instance()->get_logger()->warn("[Logout Fail] -User_id [{0:s}]", request_session->get_user_id());
        return false;
    }
}

bool friends_manager::search_user(session * request_session, std::string target_id)
{
    friends_response message;
    user_info  *target_user_info = message.mutable_friends_info();
    game_history *history = target_user_info->mutable_game_history_();
    basic_info *id = target_user_info->mutable_basic_info_();
    session *target_session = find_id_in_user_map(target_id);
    int total_games, win, lose, rating;
    total_games = win = lose = rating = 0;
    message.set_type(friends_response::SEARCH_SUCCESS);
    if ( target_session == nullptr)
    {
        if (db_connector_.get_query_user_info(target_id, win, lose, rating))
        {
            message.set_online(false);
            total_games = win + lose;
        }
        else
        {
            message.set_type(friends_response::SEARCH_FAIL);
            log_manager::get_instance()->get_logger()->warn("[Search Fail] -Req_id [{0:s}] -Target_id [{1:s}]",request_session->get_user_id(), target_id);
        }
    }
    else
    {
        message.set_online(true);
        total_games = target_session->get_battle_history();
        lose = target_session->get_lose();
        win = target_session->get_win();
        rating = target_session->get_rating();
        target_id = target_session->get_user_id();
    }

    history->set_total_games(total_games);
    history->set_win(win);
    history->set_lose(lose);
    history->set_rating_score(rating);
    id->set_id(target_id);

    log_manager::get_instance()->get_logger()->info("[Search Success] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), target_id);
    request_session->wait_send(false, message.ByteSize() + packet_header_size, packet_handler_.incode_message(message));
    return true;
}

bool friends_manager::add_friends(session * request_session, std::string target_id)
{
    friends_response message;
    user_info  *target_user_info = message.mutable_friends_info();
    game_history *history = target_user_info->mutable_game_history_();
    basic_info *id = target_user_info->mutable_basic_info_();
    history->set_total_games(0);
    history->set_win(0);
    history->set_lose(0);
    history->set_rating_score(0);
    id->set_id(target_id);
    bool ret = false;
    int friends_count = request_session->get_friends_count();
    if (friends_count < 20)
    {
        if (db_connector_.add_user_frineds_list(request_session->get_user_id(), target_id))
        {
            message.set_type(friends_response::ADD_SUCCESS);
            request_session->set_friends_count(++friends_count);
            log_manager::get_instance()->get_logger()->info("[Add Success] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), target_id);
            ret = true;
        }
        else
        {
            message.set_type(friends_response::ADD_FAIL);
            log_manager::get_instance()->get_logger()->warn("[Add Fail] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), target_id);
        }
    }
    else
    {
        message.set_type(friends_response::ADD_FAIL);
        log_manager::get_instance()->get_logger()->warn("[Add Fail] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), target_id);
    }
    request_session->wait_send(false, message.ByteSize() + packet_header_size, packet_handler_.incode_message(message));
    return ret;
}

bool friends_manager::del_friends(session * request_session, std::string target_id)
{
    friends_response message;
    user_info  *target_user_info = message.mutable_friends_info();
    game_history *history = target_user_info->mutable_game_history_();
    basic_info *id = target_user_info->mutable_basic_info_();
    history->set_total_games(0);
    history->set_win(0);
    history->set_lose(0);
    history->set_rating_score(0);
    id->set_id(target_id);
    bool ret = false;
    int friends_count = request_session->get_friends_count();
    if (friends_count > 0)
    {
        if (db_connector_.del_user_frineds_list(request_session->get_user_id(), target_id))
        {
            message.set_type(friends_response::DEL_SUCCESS);
            request_session->set_friends_count(--friends_count);
            log_manager::get_instance()->get_logger()->info("[Del Success] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), target_id);
            ret = true;
        }
        else
        {
            message.set_type(friends_response::DEL_FAIL);
            log_manager::get_instance()->get_logger()->warn("[Del Fail] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), target_id);
        }
    }
    else
    {
        message.set_type(friends_response::DEL_FAIL);
        log_manager::get_instance()->get_logger()->warn("[Del Fail] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), target_id);
    }
    request_session->wait_send(false, message.ByteSize() + packet_header_size, packet_handler_.incode_message(message));
    return ret;
}

void friends_manager::process_friends_function(session * request_session, const char * packet, const int packet_size)
{
    friends_request message;
    packet_handler_.decode_message(message, packet, packet_size);
    std::string target_id;
    target_id = message.mutable_target_info()->id();

    switch (message.type())
    {
    case friends_request::ADD:
    {
        add_friends(request_session, target_id);
        break;
    }
    case friends_request::DEL:
    {
        del_friends(request_session, target_id);
        break;
    }
    case friends_request::SEARCH:
    {
        search_user(request_session, target_id);
        return;
    }
    default:
        break;
    }
}

session* friends_manager::find_id_in_user_map(std::string target_id)
{
    user_id_map_mtx.lock();
    auto iter = user_id_map_.find(target_id);
    user_id_map_mtx.unlock();

    if (iter != user_id_map_.end())
    {
        return iter->second;
    }
    else
    {
        return nullptr;
    }
}

bool friends_manager::del_id_in_user_map(std::string target_id)
{
    bool ret = false;
    user_id_map_mtx.lock();
    auto iter = user_id_map_.find(target_id);
    if (iter != user_id_map_.end())
    {
        user_id_map_.erase(target_id);
        ret = true;
    }
    user_id_map_mtx.unlock();
    return ret;
}

bool friends_manager::add_id_in_user_map(session * request_session, std::string request_id)
{
    bool ret = false;
    user_id_map_mtx.lock();
    if (user_id_map_.find(request_id) == user_id_map_.end())
    {
        user_id_map_[request_id] = request_session;
        ret = true;
    }
    user_id_map_mtx.unlock();
    return ret;
}



