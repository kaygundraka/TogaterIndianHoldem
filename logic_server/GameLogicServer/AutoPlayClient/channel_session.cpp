#include "pre_header.h"
#include "channel_session.h"
#include "logic_session.h"
#include "game_manager.h"
#include "network_manager.h"
#include "logger.h"
#include "configurator.h"

bool channel_session::create()
{
    thread_sync sync;

    is_connected_ = false;
    
    socket_ = new tcp::socket(io_service_);

    return true;
}

bool channel_session::destroy()
{
    thread_sync sync;

    socket_->shutdown(boost::asio::socket_base::shutdown_send);
    this->disconnect();

    is_connected_ = false;

    work_thread_->join();

    if (socket_ != nullptr)
    {
        delete socket_;
        socket_ = nullptr;
    }

    return true;
}

void channel_session::handle_send(channel_server::message_type msg_type, const protobuf::Message& message)
{
    thread_sync sync;

    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    int buf_size = 0;
    buf_size = message_header_size + message.ByteSize();

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_->write_some(boost::asio::buffer(send_buf_, message_header_size + header.size), error);

    if (error)
    {
        logger::print(error.message().c_str());
        return;
    }
}

void channel_session::handle_read()
{
    while (true)
    {
        if (!is_socket_open())
            break;

        boost::system::error_code error;

        int i = 0;

        socket_->receive(boost::asio::buffer(recv_buf_), i, error);

        if (error)
        {
            logger::print(error.message().c_str());
            return;
        }

        thread_sync sync;

        MESSAGE_HEADER message_header;

        CopyMemory(&message_header, recv_buf_.begin(), message_header_size);

        switch (message_header.type)
        {
        case channel_server::JOIN_ANS:
        {
            channel_server::packet_join_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;
                
            process_packet_join_ans(message);
        }
        break;

        case channel_server::LOGOUT_ANS:
        {
            channel_server::packet_logout_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_logout_ans(message);
        }
        break;

        case channel_server::FRIENDS_ANS:
        {
            channel_server::packet_friends_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_friend_ans(message);
        }
        break;

        case channel_server::PLAY_RANK_ANS:
        {
            channel_server::packet_play_rank_game_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_rank_game_ans(message);
        }
        break;

        case channel_server::PLAY_FRIENDS_REL:
        {
            channel_server::packet_play_friends_game_rel message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_play_friend_game_rel(message);
        }
        break;

        case channel_server::MATCH_COMPLETE:
        {
            channel_server::packet_matching_complete_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_matching_complete_ans(message);
        }
        break;

        case channel_server::ERROR_MSG:
        {
            channel_server::packet_error_message message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_error_message(message);
        }
        break;

        }
    }
}

void channel_session::process_packet_join_ans(channel_server::packet_join_ans packet)
{
    thread_sync sync;

    network_logic->destroy();

    logger::print("channel_session : process_packet_join_ans");

    if (packet.success() == false)
        return;

    char temp[128] = "";
    logger::print("[friend_list]");

    for (int i = 0; i < packet.friends_list_size(); ++i)
    {
        const channel_server::basic_info& info = packet.friends_list(i);
        
        sprintf(temp, "friend id(%s)", info.id().c_str());

        logger::print(temp);
    }

    if (packet.has_history())
    {
        const channel_server::game_history& history = packet.history();

        sprintf(temp, "[player info] -> win(%d), lose(%d), rating(%d)", history.win(), history.lose(), history.rating_score());
        logger::print(temp);
    }
}

void channel_session::process_packet_logout_ans(channel_server::packet_logout_ans packet)
{
    thread_sync sync;

    logger::print("channel_session : process_packet_logout_ans");
}

void channel_session::process_packet_friend_ans(channel_server::packet_friends_ans packet)
{
    thread_sync sync;

    char temp[128] = "";

    const channel_server::user_info& friend_info = packet.friends_info();
    const channel_server::game_history& friend_history = friend_info.game_history_();
    const channel_server::basic_info& friend_basic_info = friend_info.basic_info_();
    
    friend_history.rating_score();
    friend_history.total_games();
    friend_history.win();
    friend_history.lose();
    
    switch (packet.type())
    {
    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_SEARCH_SUCCESS:
    {
        sprintf(temp, "channel_session : search_friend_success, id(%s)", packet.mutable_friends_info()->mutable_basic_info_()->id().c_str());
        logger::print(temp);

        this->send_packet_friend_req(channel_server::packet_friends_req_req_type_ADD, friend_basic_info);
    }
    break;

    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_ADD_SUCCESS:
        sprintf(temp, "channel_session : add_friend_success : id(%s)", packet.mutable_friends_info()->mutable_basic_info_()->id().c_str());
        logger::print(temp);
        break;

    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_DEL_SUCCESS:
        sprintf(temp, "channel_session : del_friend_success : id(%s)", packet.mutable_friends_info()->mutable_basic_info_()->id().c_str());
        logger::print(temp);
        break;
    }
}

void channel_session::process_packet_rank_game_ans(channel_server::packet_play_rank_game_ans packet)
{
    thread_sync sync;

    logger::print("channel_session : process_packet_rank_game_ans");
}

void channel_session::process_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel packet)
{
    thread_sync sync;

    logger::print("channel_session : process_packet_play_friend_game_rel");

    switch(packet.type())
    {
    case channel_server::packet_play_friends_game_rel::APPLY:
        if (game_mgr->send_friend_match_ == true)
        {
            game_mgr->accept_friend_match_ = false;
            game_mgr->send_friend_match_ = false;

            network_lobby->send_packet_play_friend_game_rel(
                channel_server::packet_play_friends_game_rel_req_type_DENY,
                packet.target_id().c_str()
            );

            game_mgr->state_ = game_manager::DENY;
            return;
        }

        game_mgr->accept_friend_match_ = true;
        game_mgr->friend_match_id_ = packet.target_id();
                
        break;

    case channel_server::packet_play_friends_game_rel::DENY:
        game_mgr->accept_friend_match_ = false;
        game_mgr->state_ = game_manager::DENY;
        break;
    }
}

void channel_session::process_packet_matching_complete_ans(channel_server::packet_matching_complete_ans packet)
{
    thread_sync sync;
    
    logger::print("channel_session : process_packet_matching_complete_ans");

    char temp[128] = "";

    sprintf(temp, 
        "matchiing_complete : id(%s), win(%d), lose(%d), rating(%d)",
        packet.mutable_opponent_player()->mutable_basic_info_()->id().c_str(),
        packet.mutable_opponent_player()->mutable_game_history_()->win(),
        packet.mutable_opponent_player()->mutable_game_history_()->lose(),
        packet.mutable_opponent_player()->mutable_game_history_()->rating_score()
    );

    network_mgr->set_room_key(packet.room_key());
    
    network_logic->create();
    std::string logic_server_ip;
    std::string logic_server_port;
    configurator::get_value("logic_server_ip", logic_server_ip);
    configurator::get_value("logic_server_port", logic_server_port);
    network_logic->connect(logic_server_ip, logic_server_port);
    network_logic->send_packet_enter_req(network_mgr->get_room_key(), network_mgr->get_player_key());
}

void channel_session::process_packet_error_message(channel_server::packet_error_message packet)
{
    thread_sync sync;

    logger::print("channel_session : process_packet_error_message");
}

void channel_session::send_packet_join_req(std::string key, std::string id)
{
    thread_sync sync;

    logger::print("channel_session : send_packet_join_req");

    channel_server::packet_join_req packet;

    packet.set_token(key);
    packet.set_id(id);

    this->handle_send(channel_server::JOIN_REQ, packet);
}

void channel_session::send_packet_logut_req(bool flag)
{
    thread_sync sync;

    logger::print("channel_session : send_packet_logut_req");

    channel_server::packet_logout_req packet;
    packet.set_none(flag);

    this->handle_send(channel_server::LOGOUT_REQ, packet);
}

void channel_session::send_packet_friend_req(channel_server::packet_friends_req::req_type type, channel_server::basic_info info)
{
    thread_sync sync;

    channel_server::packet_friends_req packet;
    packet.set_type(type);
    packet.mutable_target_info()->set_id(info.id());

    this->handle_send(channel_server::FRIENDS_REQ, packet);
}

void channel_session::send_packet_rank_game_req(bool flag)
{
    thread_sync sync;

    logger::print("channel_session : send_packet_rank_game_req");

    channel_server::packet_play_rank_game_req packet;
    packet.set_none(flag);

    this->handle_send(channel_server::PLAY_RANK_REQ, packet);
}

void channel_session::send_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel::req_type type, std::string target_id)
{
    thread_sync sync;

    logger::print("channel_session : send_packet_play_friend_game_rel");

    channel_server::packet_play_friends_game_rel packet;
    packet.set_type(type);
    packet.set_target_id(target_id);

    this->handle_send(channel_server::PLAY_FRIENDS_REL, packet);
}

void channel_session::send_packet_matching_confirm()
{
    thread_sync sync;

    logger::print("channel_session : send_packet_matching_confirm");

    channel_server::packet_matching_confirm packet;

    this->handle_send(channel_server::MATCH_CONFIRM, packet);
    game_mgr->state_ = game_manager::CONNECT;
    this->destroy();
}