#include "pre_header.h"
#include "chat_session.h"
#include "network_manager.h"
#include "game_manager.h"
#include "logger.h"

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
        {
            logger::print(error.message().c_str());
            return;
        }
        
        thread_sync sync;

        MESSAGE_HEADER message_header;

        CopyMemory(&message_header, recv_buf_.begin(), message_header_size);

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

    logger::print("chat_session : process_packet_verify_ans");
}

void chat_session::process_pacekt_logout_ans(chat_server::packet_logout_ans packet)
{
    thread_sync sync;

    logger::print("chat_session : process_pacekt_logout_ans");
}

void chat_session::process_packet_chat_normal(chat_server::packet_chat_normal packet)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : [chat] %s / %s", packet.user_id().c_str(), packet.chat_message().c_str());

    logger::print(temp);
}

void chat_session::process_packet_chat_whisper(chat_server::packet_chat_whisper packet)
{
    thread_sync sync;

    std::string id = "w-" + packet.user_id();
    std::string str = packet.chat_message();

    if (packet.user_id() == "Error")
    {
        id = "w - Invailed Command";
        str = "";
    }

    char temp[128] = "";
    sprintf(temp, "chat_session : [chat] %s / %s", id.c_str(), str.c_str());

    logger::print(temp);
}

void chat_session::process_packet_chat_room(chat_server::packet_chat_room packet)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : [chat] Room-%s / %s", packet.user_id().c_str(), packet.chat_message().c_str());

    logger::print(temp);
}

void chat_session::process_packet_chat_notice(chat_server::packet_chat_notice packet)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : [chat] GM-%s / %s", packet.user_id().c_str(), packet.chat_message().c_str());

    logger::print(temp);
}

void chat_session::send_packet_verify_req(std::string player_key, std::string id)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : send_packet_verify_req, player_key(%s), player_id(%s)", player_key.c_str(), id.c_str());
    logger::print(temp);

    chat_server::packet_verify_req packet;
    packet.set_key_string(player_key);
    packet.set_value_user_id(id);

    this->handle_send(chat_server::VERIFY_REQ, packet);
}

void chat_session::send_packet_logout_req(std::string player_id)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : send_packet_logout_req, player_id(%s)", player_id.c_str());
    logger::print(temp);
    
    chat_server::packet_logout_req packet;
    packet.set_user_id(player_id);
    
    this->handle_send(chat_server::LOGOUT_REQ, packet);
}

void chat_session::send_packet_enter_match_ntf(std::string target_id)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : send_packet_enter_match_ntf, target_id(%s)", target_id.c_str());
    logger::print(temp);
    
    chat_server::packet_enter_match_ntf packet;

    packet.set_opponent_id(target_id);

    this->handle_send(chat_server::ENTER_MATCH_NTF, packet);
}

void chat_session::send_packet_leave_match_ntf()
{
    thread_sync sync;

    logger::print("chat_session : send_packet_leave_match_ntf");
    
    chat_server::packet_leave_match_ntf packet;

    this->handle_send(chat_server::LEAVE_MATCH_NTF, packet);
}

void chat_session::send_packet_chat_normal(std::string id, std::string message)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : send_packet_chat_normal, id(%s), msg(%s)", id.c_str(), message.c_str());
    logger::print(temp);
    
    chat_server::packet_chat_normal packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);

    this->handle_send(chat_server::NORMAL, packet);
}

void chat_session::send_packet_chat_whisper(std::string id, std::string target_id, std::string message)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : send_packet_chat_whisper, id(%s), target_id(%s), msg(%s)", id.c_str(), message.c_str());
    logger::print(temp);

    chat_server::packet_chat_whisper packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);
    packet.set_target_id(target_id);

    this->handle_send(chat_server::WHISPER, packet);
}

void chat_session::send_packet_chat_room(std::string id, std::string message)
{
    thread_sync sync;

    char temp[128] = "";
    sprintf(temp, "chat_session : send_packet_chat_whisper, id(%s), msg(%s)", id.c_str(), message.c_str());
    logger::print(temp);

    chat_server::packet_chat_room packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);

    this->handle_send(chat_server::ROOM, packet);
}