#include "pre_headers.h"
#include "connected_session.h"
#include "redis_connector.h"
#include "log.h"

connected_session::connected_session(boost::asio::io_service& io_service) : socket_(io_service), safe_disconnect_(false), is_accept_(false)
{
    player_key_ = "";
    room_key_ = "";
    enter_room_ = true;
    create_room_ = false;

    is_remain = false;
    pre_remain_size = 0;
}

void connected_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
{
    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_.write_some(boost::asio::buffer(send_buf_, message_header_size + header.size), error);

    if (error)
    {
        Log::WriteLog(_T("%s"), error.message().c_str());
        shut_down();
    }
}

void connected_session::shut_down()
{
    if (socket_.is_open())
    {
        socket_.shutdown(boost::asio::socket_base::shutdown_receive);
        socket_.close();
    }
}

std::string connected_session::get_player_key()
{
    return player_key_;
}

std::string connected_session::get_room_key()
{
    return room_key_;
}

void connected_session::handle_read(const boost::system::error_code& error, size_t buf_size)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(temp_buf),
            boost::bind(&connected_session::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

        if (is_remain)
            CopyMemory(recv_buf_.begin(), remain_buf.begin(), pre_remain_size);
        
        CopyMemory(recv_buf_.begin() + pre_remain_size, temp_buf.begin(), buf_size);
        
        int process_size = 0;
        int remain_size = buf_size + pre_remain_size;

        do
        {
            if (message_header_size > remain_size)
            {
                CopyMemory(remain_buf.begin(), recv_buf_.begin() + process_size, remain_size);
                pre_remain_size = remain_size;
                is_remain = true;
                break;
            }

            MESSAGE_HEADER message_header;
            ZeroMemory(&message_header, sizeof(message_header));
            CopyMemory(&message_header, recv_buf_.begin() + process_size, message_header_size);
            
            if (message_header_size + message_header.size > remain_size)
            {
                CopyMemory(remain_buf.begin(), recv_buf_.begin() + process_size, remain_size);
                pre_remain_size = remain_size;
                is_remain = true;
                break;
            }

            CopyMemory(packet_buf.begin(), recv_buf_.begin() + process_size, message_header.size + message_header_size);

            process_size += message_header_size + message_header.size;
            remain_size -= process_size;

            switch (message_header.type)
            {
            case logic_server::ENTER_REQ:
            {
                logic_server::packet_enter_req message;
                
                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_enter_req(message);
            }
            break;

            case logic_server::GAME_STATE_NTF:
            {
                logic_server::packet_game_state_ntf message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                porcess_packet_game_state_ntf(message);
            }
            break;

            case logic_server::PROCESS_TURN_ANS:
            {
                logic_server::packet_process_turn_ans message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_process_turn_ans(message);
            }
            break;

            case logic_server::DISCONNECT_ROOM:
            {
                logic_server::packet_disconnect_room_ntf message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_disconnect_room_ntf(message);
            }
            break;

            case logic_server::ECHO_NTF:
            {
                logic_server::packet_echo_ntf message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_echo_ntf(message);
            }
            break;
            }

            if (remain_size == 0)
            {
                is_remain = false;
                pre_remain_size = 0;
            }

        } while (remain_size > 0);
    }
    else
    {
        if (error == boost::asio::error::eof)
        {
            Log::WriteLog(_T("error_eof : %s"), error.message().c_str());

            this->shut_down();

        }
        else if (error == boost::asio::error::shut_down)
        {
            Log::WriteLog(_T("shut_down_socket: %s"), error.message().c_str());

            this->shut_down();
        }
        else if (error.value() == 10054)
        {
            Log::WriteLog(_T("shut_down_socket: %s"), error.message().c_str());

            this->shut_down();
        }
    }
}

bool connected_session::is_connected()
{
    if (socket_.is_open())
        return true;

    return false;
}

connected_session::pointer connected_session::create(boost::asio::io_service& io_service)
{
    return connected_session::pointer(new connected_session(io_service));
}

tcp::socket& connected_session::get_socket()
{
    return socket_;
}

void connected_session::start()
{
    socket_.async_read_some(boost::asio::buffer(temp_buf),
        boost::bind(&connected_session::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));

    boost::asio::socket_base::keep_alive option;
    socket_.set_option(option);

    is_accept_ = true;
}

void connected_session::set_safe_disconnect(bool safe_disconnect)
{
    safe_disconnect_ = safe_disconnect;
}

bool connected_session::is_safe_disconnect()
{
    return safe_disconnect_;
}

bool connected_session::is_in_room()
{
    return enter_room_;
}

void connected_session::set_room_state(bool is_enter)
{
    enter_room_ = is_enter;
}

bool connected_session::accept_client()
{
    return is_accept_;
}