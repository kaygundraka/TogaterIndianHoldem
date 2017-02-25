#include "channel_server.h"


tcp_server::tcp_server(boost::asio::io_service & io_service, friends_manager& friends, match_manager& match, packet_handler& packet_handler)
    : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
    , friends_manager_(friends)
    , match_manager_(match)
    , packet_handler_(packet_handler)
    , connections(0)
    , match_counts(0)
    , room_key_del(0)
{
    accepting_flag_ = false;
    load_server_config();
    acceptor_ = boost::asio::ip::tcp::acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
}

tcp_server::~tcp_server()
{
    for (size_t i = 0; i < session_list_.size(); ++i)
    {
        if (session_list_[i]->get_socket().is_open())
        {
            session_list_[i]->get_socket().close();
        }
        delete session_list_[i];
    }
}

void tcp_server::init()
{
    for (int i = 0; i < max_session_count; ++i)
    {
        session *p_session = new session(i, acceptor_.get_io_service(), this, max_token_size, max_buffer_len);
        session_list_.push_back(p_session);
        session_queue_.push_back(i);
    }
}

void tcp_server::start()
{
    for (int i = 1; i < max_thread; ++i)
    {
        wait_accept();
        boost::thread t(boost::bind(&boost::asio::io_service::run, &acceptor_.get_io_service()));
    }
    wait_accept();
    std::cout << "\n\n[TCP Server START] \n\n -Thread:" << max_thread << " -Port:" << port << " -Max Session:" << max_session_count << "-Recv Buffer Size:" << max_buffer_len<<std::endl;
    log_manager::get_instance()->get_logger()->info("\n\n[TCP Server START] \n\n -Thread:{0:d} -Port:{1:d} -Max Session:{2:d} -Recv Buffer Size:{3:d}", max_thread, port, max_session_count, max_buffer_len);
}

void tcp_server::close_session(const int n_session_id)
{
    session *request_session = session_list_[n_session_id];
    status request_status = request_session->get_status();

    if (request_status == status::LOGIN)
    {
        friends_manager_.del_redis_token(request_session->get_token());
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->warn("[Close] -Status:LOGIN -User_id:{0:s}", request_session->get_user_id());
    }
    else if(request_status == status::LOGOUT)
    {
        log_manager::get_instance()->get_logger()->info("[Close] -Status:LOGOUT -User_id:{0:s}", request_session->get_user_id());
    }
    else if (request_status == status::MATCH_CONFIRM)
    {
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->info("[Close] -Status:MATCH_CONFIRM -User_id:{0:s}", request_session->get_user_id());
    }
    else if (request_status == status::MATCH_COMPLETE)
    {
        friends_manager_.del_redis_token(request_session->get_room_key());
        ++room_key_del;
        friends_manager_.del_redis_token(request_session->get_token());
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->info("[Close] -Status:MATCH_COMPLETE -User_id:{0:s}", request_session->get_user_id());
    }
    else if (request_status == status::MATCH_RECVER)
    {
        friends_manager_.del_redis_token(request_session->get_token());
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->warn("[Close] -Status:MATCH_RECVER -User_id:{0:s}", request_session->get_user_id());
    }
    else if (request_status == status::MATCH_REQUEST)
    {
        friends_manager_.del_redis_token(request_session->get_token());
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->warn("[Close] -Status:MATCH_REQUEST -User_id:{0:s}", request_session->get_user_id());
    }
    else if(request_status == status::CONN)
    {
        log_manager::get_instance()->get_logger()->warn("[Close] -Status:CONN -Session_id:{0:d}", n_session_id);
    }
    else
    {
        log_manager::get_instance()->get_logger()->critical("[Close] -Status:WAIT -User_id:{0:s}", request_session->get_user_id());
        return;
    }
    
    request_session->set_status(status::WAIT);

    request_session->get_socket().close();
    --connections;
    session_queue_mtx.lock();
    session_queue_.push_back(n_session_id);
    if (accepting_flag_ == false)
    {
        wait_accept();
    }
    session_queue_mtx.unlock();
}

void tcp_server::process_packet(const int n_session_id, const char * p_data)
{
    packet_header *p_header = (packet_header *)p_data;
    session *request_session = get_session(n_session_id);
    switch (p_header->type)
    {
    case message_type::FRIENDS_REQ:
    {
        friends_manager_.process_friends_function(get_session(n_session_id), &p_data[packet_header_size], p_header->size);
        break;
    }
    case message_type::PLAY_FRIENDS_REL:
    {
        match_manager_.process_matching_with_friends(request_session, &p_data[packet_header_size], p_header->size);
        break;
    }
    case message_type::PLAY_RANK_REQ:
    {
        match_manager_.process_matching(request_session, &p_data[packet_header_size], p_header->size);
        break;
    }
    case message_type::JOIN_REQ:
    {
        friends_manager_.lobby_login_process(request_session, &p_data[packet_header_size], p_header->size);
        break;
    }
    case message_type::LOGOUT_REQ:
    {
        friends_manager_.lobby_logout_process(request_session, &p_data[packet_header_size], p_header->size);
        break;
    }
    case message_type::MATCH_CONFIRM:
    {
        if (match_manager_.process_match_confirm(request_session, &p_data[packet_header_size], p_header->size))
        {
            close_session(n_session_id);
        }
        break;
    }
    case message_type::ERROR_MSG:
    {
        process_config(get_session(n_session_id), &p_data[packet_header_size], p_header->size);
        break;
    }
    default:
        break;
    }
}

void tcp_server::process_config(session *request_session, const char *packet, const int data_size)
{
    error_report message;
    packet_handler_.decode_message(message, packet, data_size);
    char cmd[50] = { 0, };
    if (message.error_string() == "get session count")
    {
        sprintf(cmd, "session count : %d", connections);
        message.set_error_string(cmd);
    }
    else if(message.error_string() == "alive")
    {
        message.set_error_string("yes");
    }
    else if (message.error_string() == "get room key del")
    {
        sprintf(cmd, "room key del : %d", room_key_del);
        message.set_error_string(cmd);
    }
    else
    {
        message.set_error_string("Not Found Command");
    }
    request_session->wait_send(false, message.ByteSize() + packet_header_size, packet_handler_.incode_message(message));
}

bool tcp_server::rematching_request(session * request_session)
{
    return match_manager_.rematching_start(request_session);
}

bool tcp_server::wait_accept()
{
    session_queue_mtx.lock();
   
    if (session_queue_.empty())
    {
        accepting_flag_ = false;
        session_queue_mtx.unlock();
        return false;
    }
    accepting_flag_ = true;
    int n_session_id = session_queue_.front();
    session_queue_.pop_front();
    
    session_queue_mtx.unlock();
    
    acceptor_.async_accept(session_list_[n_session_id]->get_socket(), boost::bind(&tcp_server::handle_accept, this, session_list_[n_session_id], boost::asio::placeholders::error));

    return true;
}

void tcp_server::handle_accept(session * p_session, const boost::system::error_code & error)
{
    if (!error)
    {
        p_session->init();
        p_session->wait_receive();
        p_session->set_status(status::CONN);
        ++connections;
        log_manager::get_instance()->get_logger()->info("[Session Connect] -Session_id [{0:d}] -Session_count [{1:d}]",p_session->get_session_id(),connections);
        wait_accept();
    }
    else
    {
        log_manager::get_instance()->get_logger()->warn("[Accept Error] -No [{0:d}] -Error Message [{1:s}]",error.value(), error.message());
    }
}

void tcp_server::load_server_config()
{
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_THREAD", max_thread);
    config::get_instance()->get_value("SERVER_CONFIG", "PORT", port);
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_BUFFER_LEN", max_buffer_len);
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_TOKEN_SIZE", max_token_size);
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_SESSION_COUNT", max_session_count);
}