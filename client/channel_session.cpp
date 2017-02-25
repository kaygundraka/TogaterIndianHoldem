#include "channel_session.h"
#include "logic_session.h"
#include "game_manager.h"
#include "chat_session.h"
#include "network_manager.h"

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

    //socket_->shutdown(boost::asio::socket_base::send);
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

    memcpy(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_->write_some(boost::asio::buffer(send_buf_, message_header_size + header.size), error);
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
            return;

        thread_sync sync;

        MESSAGE_HEADER message_header;

        memcpy(&message_header, recv_buf_.begin(), message_header_size);

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

    if (packet.success() == false)
        return;

    for (int i = 0; i < packet.friends_list_size(); ++i)
    {
        const channel_server::basic_info& info = packet.friends_list(i);
        
        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::add_friend_in_list,
                game_mgr,
                info.id()
            )
        );
    }

    if (packet.has_history())
    {
        const channel_server::game_history& history = packet.history();

        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::set_history,
                game_mgr,
                history.win(),
                history.lose(),
                history.rating_score()
            )
        );

        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::set_tear,
                game_mgr,
                history.rating_score()
            )
        );

        network_mgr->set_player_history(history);
    }
}

void channel_session::process_packet_logout_ans(channel_server::packet_logout_ans packet)
{
    thread_sync sync;

}

void channel_session::process_packet_friend_ans(channel_server::packet_friends_ans packet)
{
    thread_sync sync;

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
        channel_server::basic_info info;
        info.set_id(game_mgr->friend_text_field->getString());
        this->send_packet_friend_req(channel_server::packet_friends_req_req_type_ADD, info);
    }
    break;

    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_ADD_SUCCESS:
        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::add_friend_in_list,
                game_mgr,
                packet.mutable_friends_info()->mutable_basic_info_()->id()
            )
        );
        break;

    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_DEL_SUCCESS:
        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::del_friend_in_list,
                game_mgr,
                packet.mutable_friends_info()->mutable_basic_info_()->id()
            )
        );
        break;
    }
}

void channel_session::process_packet_rank_game_ans(channel_server::packet_play_rank_game_ans packet)
{
    thread_sync sync;

}

void channel_session::process_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel packet)
{
    thread_sync sync;

    switch(packet.type())
    {
    case channel_server::packet_play_friends_game_rel::APPLY:
       /* if (game_mgr->send_friend_match_ == true || game_mgr->accept_friend_match_ == true)
        {
            game_mgr->accept_friend_match_ = false;
            game_mgr->send_friend_match_ = false;

            network_lobby->send_packet_play_friend_game_rel(
                channel_server::packet_play_friends_game_rel_req_type_DENY,
                packet.target_id()
            );

            game_mgr->lobby_scene_->hide_friend_match_pop_up();

            cocos2d::CCDirector::getInstance()->popScene();

            return;
        }*/

        game_mgr->friend_match_id_ = packet.target_id();

        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                lobby_scene::show_friend_match_pop_up,
                game_mgr->lobby_scene_
            )
        );

        break;

    case channel_server::packet_play_friends_game_rel::DENY:
        game_mgr->accept_friend_match_ = false;
        game_mgr->send_friend_match_ = false;
                
        cocos2d::CCDirector::getInstance()->popScene();
        break;
    }
}

void channel_session::process_packet_matching_complete_ans(channel_server::packet_matching_complete_ans packet)
{
    thread_sync sync;

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            chat_session::send_packet_enter_match_ntf,
            network_chat,
            packet.mutable_opponent_player()->mutable_basic_info_()->id()
        )
    );
    
    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::set_opponent_info,
            game_mgr,
            packet.mutable_opponent_player()->mutable_basic_info_()->id(),
            packet.mutable_opponent_player()->mutable_game_history_()->win(),
            packet.mutable_opponent_player()->mutable_game_history_()->lose(),
            packet.mutable_opponent_player()->mutable_game_history_()->rating_score()
        )
    );

    network_mgr->set_room_key(packet.room_key());

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(game_manager::start_game, game_mgr)
    );
    
    send_packet_matching_confirm();
}

void channel_session::process_packet_error_message(channel_server::packet_error_message packet)
{
    thread_sync sync;
}

void channel_session::send_packet_join_req(std::string key, std::string id)
{
    thread_sync sync;

    channel_server::packet_join_req packet;

    packet.set_token(key);
    packet.set_id(id);

    this->handle_send(channel_server::JOIN_REQ, packet);
}

void channel_session::send_packet_logut_req(bool flag)
{
    thread_sync sync;

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

    channel_server::packet_play_rank_game_req packet;
    packet.set_none(flag);

    this->handle_send(channel_server::PLAY_RANK_REQ, packet);
}

void channel_session::send_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel::req_type type, std::string target_id)
{
    thread_sync sync;

    channel_server::packet_play_friends_game_rel packet;
    packet.set_type(type);
    packet.set_target_id(target_id);

    this->handle_send(channel_server::PLAY_FRIENDS_REL, packet);
}

void channel_session::send_packet_matching_confirm()
{
    thread_sync sync;

    channel_server::packet_matching_confirm packet;

    this->handle_send(channel_server::MATCH_CONFIRM, packet);

    this->destroy();
}