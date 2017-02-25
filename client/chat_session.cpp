#include "chat_session.h"
#include "game_manager.h"
#include "network_manager.h"

bool chat_session::create()
{
    thread_sync sync;

    is_connected_ = false;

    socket_ = new tcp::socket(io_service_);

    return true;
}

bool chat_session::destroy()
{
    thread_sync sync;

    is_connected_ = false;

    work_thread_->join();

    if (socket_ != nullptr)
        delete socket_;

    return true;
}

void chat_session::handle_send(chat_server::message_type msg_type, const protobuf::Message& message)
{
    thread_sync sync;

    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    int buf_size = 0;
    buf_size = message_header_size + message.ByteSize();

    memcpy(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    socket_->write_some(boost::asio::buffer(send_buf_, message_header_size + header.size));
}

void chat_session::handle_read()
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
        case chat_server::VERIFY_ANS:
        {
            chat_server::packet_verify_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_verify_ans(message);
        }
        break;

        case chat_server::LOGOUT_ANS:
        {
            chat_server::packet_logout_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_pacekt_logout_ans(message);
        }
        break;
        
        case chat_server::NORMAL:
        {
            chat_server::packet_chat_normal message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_chat_normal(message);
        }
        break;

        case chat_server::WHISPER:
        {
            chat_server::packet_chat_whisper message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_chat_whisper(message);
        }
        break;

        case chat_server::ROOM:
        {
            chat_server::packet_chat_room message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_chat_room(message);
        }
        break;

        case chat_server::NOTICE:
        {
            chat_server::packet_chat_notice message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_chat_notice(message);
        }
        break;

        }
    }
}

void chat_session::process_packet_verify_ans(chat_server::packet_verify_ans packet)
{
    thread_sync sync;
}

void chat_session::process_pacekt_logout_ans(chat_server::packet_logout_ans packet)
{
    thread_sync sync;
}

void chat_session::process_packet_chat_normal(chat_server::packet_chat_normal packet)
{
    thread_sync sync;

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::update_chat, game_mgr,
            packet.user_id(), packet.chat_message(), game_manager::CHAT_TYPE::NORMAL
        )
    );
}

void chat_session::process_packet_chat_whisper(chat_server::packet_chat_whisper packet)
{
    thread_sync sync;

    std::string id = "[w]" + packet.user_id();
    std::string str = packet.chat_message();

    if (packet.user_id() == "Error")
    {
        id = "[w] - Invaild Command";
        str = "";
    }

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::update_chat, game_mgr,
            id, str, game_manager::CHAT_TYPE::WHISPER
        )
    );
}

void chat_session::process_packet_chat_room(chat_server::packet_chat_room packet)
{
    thread_sync sync;

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::update_chat, game_mgr,
            packet.user_id(), packet.chat_message(), game_manager::CHAT_TYPE::NORMAL
        )
    );
}

void chat_session::process_packet_chat_notice(chat_server::packet_chat_notice packet)
{
    thread_sync sync;
    
    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::update_chat, game_mgr,
            "[GM]" + packet.user_id(), packet.chat_message(), game_manager::CHAT_TYPE::NOTICE
        )
    );
}

void chat_session::send_packet_verify_req(std::string player_key, std::string id)
{
    thread_sync sync;

    chat_server::packet_verify_req packet;
    packet.set_key_string(player_key);
    packet.set_value_user_id(id);

    this->handle_send(chat_server::VERIFY_REQ, packet);
}

void chat_session::send_packet_logout_req(std::string player_id)
{
    thread_sync sync;

    chat_server::packet_logout_req packet;
    packet.set_user_id(player_id);
    
    this->handle_send(chat_server::LOGOUT_REQ, packet);
}

void chat_session::send_packet_enter_match_ntf(std::string target_id)
{
    thread_sync sync;
    
    chat_server::packet_enter_match_ntf packet;

    packet.set_opponent_id(target_id);

    this->handle_send(chat_server::ENTER_MATCH_NTF, packet);
}

void chat_session::send_packet_leave_match_ntf()
{
    thread_sync sync;

    chat_server::packet_leave_match_ntf packet;

    this->handle_send(chat_server::LEAVE_MATCH_NTF, packet);
}

void chat_session::send_packet_chat_normal(std::string id, std::string message)
{
    thread_sync sync;

    chat_server::packet_chat_normal packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);

    this->handle_send(chat_server::NORMAL, packet);
}

void chat_session::send_packet_chat_whisper(std::string id, std::string target_id, std::string message)
{
    thread_sync sync;

    chat_server::packet_chat_whisper packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);
    packet.set_target_id(target_id);

    this->handle_send(chat_server::WHISPER, packet);
}

void chat_session::send_packet_chat_room(std::string id, std::string message)
{
    thread_sync sync;

    chat_server::packet_chat_room packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);

    this->handle_send(chat_server::ROOM, packet);
}