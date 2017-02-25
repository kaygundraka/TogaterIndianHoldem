#include "tcp_client.h"


// ---------- public ----------
tcp_client::tcp_client(boost::asio::io_service& io_service)
    :io_service_(io_service), socket_(io_service)
{
    InitializeCriticalSectionAndSpinCount(&lock_, 4000);
    is_login_ = true;
}

tcp_client::~tcp_client()
{
    // CRITICAL SECTION START
    EnterCriticalSection(&lock_);

    while (send_data_queue_.empty() == false)
    {
        delete[] send_data_queue_.front();
        send_data_queue_.pop_front();
    }

    LeaveCriticalSection(&lock_);
    // CRITICAL SECTION END

    DeleteCriticalSection(&lock_);
}

void tcp_client::connect(boost::asio::ip::tcp::endpoint endpoint)
{
    packet_buffer_mark_ = 0;

    socket_.async_connect(endpoint,
        boost::bind(&tcp_client::handle_connect, this,
            boost::asio::placeholders::error)
    );
}

void tcp_client::close()
{
    if (socket_.is_open())
        socket_.close();
}


void tcp_client::post_send(const bool immediate, const int size, BYTE* data)
{
    BYTE* send_data = nullptr;

    // CRITICAL SECTION START
    EnterCriticalSection(&lock_);

    if (immediate == false)
    {
        send_data = data;
        send_data_queue_.push_back(send_data);
    }
    else
        send_data = data;

    if (immediate || send_data_queue_.size() < 2)
    {
        boost::asio::async_write(socket_, boost::asio::buffer(send_data, size),
            boost::bind(&tcp_client::handle_write, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
        );
    }

    LeaveCriticalSection(&lock_);
    // CRITICAL SECTION END
}

void tcp_client::post_verify_req()
{
    chat_server::packet_verify_req verify_message;
    verify_message.set_key_string(key_);
    verify_message.set_value_user_id(user_id_);

    MESSAGE_HEADER header;

    header.size = verify_message.ByteSize();
    header.type = chat_server::VERIFY_REQ;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    verify_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_.write_some(boost::asio::buffer(send_buffer_.begin(), message_header_size + header.size), error);
    size_t size = socket_.read_some(boost::asio::buffer(receive_buffer_), error);

    process_packet(size);
}

void tcp_client::post_enter_match(std::string opponent_id)
{
    chat_server::packet_enter_match_ntf enter_match_message;
    enter_match_message.set_opponent_id(opponent_id);

    MESSAGE_HEADER header;

    header.size = enter_match_message.ByteSize();
    header.type = chat_server::ENTER_MATCH_NTF;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    enter_match_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer_.begin());
}

void tcp_client::post_leave_match()
{
    chat_server::packet_leave_match_ntf leave_match_message;

    MESSAGE_HEADER header;

    header.size = leave_match_message.ByteSize();
    header.type = chat_server::LEAVE_MATCH_NTF;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    leave_match_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer_.begin());
}


void tcp_client::post_normal(std::string message)
{
    chat_server::packet_chat_normal normal_message;
    normal_message.set_user_id(user_id_);
    normal_message.set_chat_message(message);

    MESSAGE_HEADER header;

    header.size = normal_message.ByteSize();
    header.type = chat_server::NORMAL;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    normal_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer_.begin());
}

void tcp_client::post_whisper(std::string target_id, std::string message)
{
    chat_server::packet_chat_whisper whisper_message;
    whisper_message.set_user_id(user_id_);
    whisper_message.set_target_id(target_id);
    whisper_message.set_chat_message(message);

    MESSAGE_HEADER header;

    header.size = whisper_message.ByteSize();
    header.type = chat_server::WHISPER;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    whisper_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer_.begin());
}

void tcp_client::post_room(std::string message)
{
    chat_server::packet_chat_room room_message;
    room_message.set_user_id(user_id_);
    room_message.set_chat_message(message);

    MESSAGE_HEADER header;

    header.size = room_message.ByteSize();
    header.type = chat_server::ROOM;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    room_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer_.begin());
}

void tcp_client::post_notice(std::string message)
{
    chat_server::packet_chat_notice notice_message;
    notice_message.set_user_id(user_id_);
    notice_message.set_chat_message(message);

    MESSAGE_HEADER header;

    header.size = notice_message.ByteSize();
    header.type = chat_server::NOTICE;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    notice_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer_.begin());
}


// ---------- private ----------
void tcp_client::post_receive()
{
    socket_.async_read_some(
        boost::asio::buffer(receive_buffer_),
        boost::bind(&tcp_client::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
    );
}

void tcp_client::handle_connect(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "Server connection successed" << std::endl;

        post_receive();
    }
    else
        std::cout << "Server connection failed. error No: " << error.value() << " error Message: " << error.message() << std::endl;
}

void tcp_client::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
    // CRITICAL SECTION START
    EnterCriticalSection(&lock_);

    send_data_queue_.pop_front();

    BYTE* data = nullptr;

    if (send_data_queue_.empty() == false)
        data = send_data_queue_.front();
    
    LeaveCriticalSection(&lock_);
    // CRITICAL SECTION END

    
    if (data != nullptr)
    {
        MESSAGE_HEADER* header = (MESSAGE_HEADER*)data;
        post_send(true, header->size + message_header_size, data);
    }
}

void tcp_client::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        if (process_packet(bytes_transferred))
            post_receive();
        else
            return;
    }
    else
    {
        if (error == boost::asio::error::eof)
            std::cout << "Disconnected with server" << std::endl;
        else
            std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;

        close();
    }
}

bool tcp_client::process_packet(const int size)
{
    CopyMemory(&packet_buffer_, receive_buffer_.data(), size);

    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)packet_buffer_.begin();

    switch (message_header->type)
    {
    case chat_server::VERIFY_ANS:
        {
            chat_server::packet_verify_ans verify_message;
        
            verify_message.ParseFromArray(packet_buffer_.begin() + message_header_size, message_header->size);

            if (!verify_message.is_successful())
            {
                std::cout << "Cookie or ID is incorrect." << std::endl;
                return false;
            }
            else
                is_login_ = true;
        }
    break;

    case chat_server::LOGOUT_ANS:
        {
            chat_server::packet_logout_ans logout_message;

            logout_message.ParseFromArray(packet_buffer_.begin() + message_header_size, message_header->size);

            if (!logout_message.is_successful())
            {
                std::cout << "Logout Failed" << std::endl;
                return true;
            }
            else
                return false;
        }
        break;



    case chat_server::NORMAL:
        {
            //chat_server::packet_chat_normal normal_message;
        
            //normal_message.ParseFromArray(packet_buffer_.begin() + message_header_size, message_header->size);

            //std::cout << normal_message.user_id() << "> ";
            //std::cout << normal_message.chat_message() << std::endl;

            //TEMP_COUNT++;
            //if (TEMP_COUNT >= 100)
            //{
            //    std::cout << user_id_ << " / 100" << std::endl;
            //    TEMP_COUNT = 0;
            //}
        }
    break;

    case chat_server::WHISPER:
        break;

    case chat_server::ROOM:
        break;

    case chat_server::NOTICE:
    {
        chat_server::packet_chat_notice notice_message;

        notice_message.ParseFromArray(packet_buffer_.begin() + message_header_size, message_header->size);

        std::cout << notice_message.user_id() << "> ";
        std::cout << notice_message.chat_message() << std::endl;
    }
        break;

    default:
        std::cout << "type: " << message_header->type << std::endl;
        break;
    }
    
    return true;
}