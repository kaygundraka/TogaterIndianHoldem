#include "log_manager.h"
#include "redis_connector.h"

#include "tcp_server.h"
#include "tcp_session.h"


// ---------- public ----------
tcp_session::tcp_session(int session_id, boost::asio::io_service& io_service, tcp_server* server)
    :socket_(io_service), session_id_(session_id), server_(server), user_key_(""), user_id_("")
{
}

tcp_session::~tcp_session()
{
}

void tcp_session::post_send(const bool immediate, const int size, BYTE* data)
{
    if (immediate == false)
        send_data_queue_.push_back(data);
    
    if (immediate == false && send_data_queue_.size() > 1)
        return;

    boost::asio::async_write(socket_, boost::asio::buffer(data, size),
        boost::bind(&tcp_session::handle_write, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
    );
}

void tcp_session::post_receive()
{
    socket_.async_read_some(boost::asio::buffer(receive_buffer_),
        boost::bind(&tcp_session::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
    );
}


// ---------- private ----------
void tcp_session::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)send_data_queue_.front();
    

    switch (message_header->type)
    {
    case chat_server::VERIFY_ANS:
        {
            chat_server::packet_verify_ans verify_message;
            verify_message.ParseFromArray(send_data_queue_.front() + message_header_size, message_header->size);

            if (socket_.is_open() && !(verify_message.is_successful()))
                server_->get_io_service().post(server_->strand_close_.wrap(boost::bind(&tcp_server::close_session, server_, session_id_)));
        }
        break;

    case chat_server::LOGOUT_ANS:
        {
            chat_server::packet_logout_ans logout_message;
            logout_message.ParseFromArray(send_data_queue_.front() + message_header_size, message_header->size);

            if (socket_.is_open() && logout_message.is_successful())
                server_->get_io_service().post(server_->strand_close_.wrap(boost::bind(&tcp_server::close_session, server_, session_id_)));
        }
        break;

    }

    send_data_queue_.pop_front();
    if (send_data_queue_.empty() == false)
    {
        BYTE* data = send_data_queue_.front();
        MESSAGE_HEADER* packet = (MESSAGE_HEADER*)data;
        post_send(true, packet->size + message_header_size, data);
    }
}

void tcp_session::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        server_->get_io_service().post(server_->strand_receive_.wrap(boost::bind(&tcp_server::process_packet, server_, session_id_, bytes_transferred, receive_buffer_.begin())));

        post_receive();
    }
    else
    {
        // UTF-8 변환
        std::string err_msg = CW2A(CA2W(error.message().c_str()), CP_UTF8);

        // Error.10054 : 현재 연결은 원격 호스트에 의해 강제로 끊겼습니다.
        if (error == boost::asio::error::eof || error.value() == 10054)
        {
            LOG_INFO << "Disconnected with client. " << err_msg;
            server_->get_io_service().post(server_->strand_close_.wrap(boost::bind(&tcp_server::close_session, server_, session_id_)));
        }
        else
            LOG_WARN << "handle_receive() - Error Message: " << err_msg;
    }
}

bool tcp_session::set_user_key(std::string user_key)
{
    user_key_ = user_key;

    if (user_key_ == "")
        return false;
    else
        return true;
}

bool tcp_session::set_user_id(std::string user_id)
{
    user_id_ = user_id;

    if (user_id_ == "")
        return false;
    else
        return true;
}

bool tcp_session::set_opponent_session(tcp_session* opponent_session)
{
    opponent_session_ = opponent_session;

    if (opponent_session_ == nullptr)
        return false;
    else
        return true;
}

bool tcp_session::set_status(user_status status)
{
    status_ = status;

    if (status_ != status)
        return false;
    else
        return true;
}