#include "config.h"
#include "log_manager.h"
#include "redis_connector.h"

#include "tcp_server.h"



// ---------- public ----------
tcp_server::tcp_server(boost::asio::io_service& io_service, int server_port, int master_buffer_len)
    :io_service_(io_service),
        acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), server_port)),
        strand_accept_(io_service), strand_close_(io_service), strand_receive_(io_service),  strand_send_(io_service)
{
    is_accepting_ = false;
    master_data_queue_.set_capacity(master_buffer_len);

    for (int i = 0; i < 5; i++)
        reserved_message_size[i] = 0;

    if (!make_reserved_message())
    {
        LOG_ERROR << "tcp_server() - Reserved message making failed.";
        std::cout << "Reserved message making failed." << std::endl;
    }
}

tcp_server::~tcp_server()
{
    for (int i = 0; i < session_list_.size(); i++)
    {
        if (session_list_[i]->get_socket().is_open())
            session_list_[i]->get_socket().close();
        
        delete session_list_[i];
    }
}

bool tcp_server::init(const int max_session_count)
{
    try
    {
        for (int i = 0; i < max_session_count; i++)
        {
            tcp_session* session = new tcp_session(i, acceptor_.get_io_service(), this);
            session_list_.push_back(session);
            session_queue_.push_back(i);
        }

        return true;
    }
    catch (...)
    {
        LOG_ERROR << "init() - Server init failed.";
        std::cout << "Server init failed." << std::endl;

        return false;
    }
}

bool tcp_server::start()
{ 
    try
    {
        LOG_INFO << "Server Start";
        std::cout << "Server Start" << std::endl;

        // There are 8 threads.
        for (int i = 0; i < 8; i++)
            io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));

        return true;
    }
    catch (...)
    {
        LOG_ERROR << "start() - Server start failed.";
        std::cout << "Server start failed." << std::endl;

        return false;
    }
}
 
bool tcp_server::make_reserved_message()
{
    // VERIFY TRUE
    // -------------------------------------------------------------
    {
        chat_server::packet_verify_ans verify_message;
        verify_message.set_is_successful(true);

        MESSAGE_HEADER header;

        header.size = verify_message.ByteSize();
        header.type = chat_server::VERIFY_ANS;

        memcpy(reserved_message[VERIFY_TRUE].begin(), (void*)&header, message_header_size);
        verify_message.SerializeToArray(reserved_message[VERIFY_TRUE].begin() + message_header_size, header.size);

        reserved_message_size[VERIFY_TRUE] = message_header_size + header.size;
    }

    // VERIFY FALSE
    // -------------------------------------------------------------
    {
        chat_server::packet_verify_ans verify_message;
        verify_message.set_is_successful(false);

        MESSAGE_HEADER header;

        header.size = verify_message.ByteSize();
        header.type = chat_server::VERIFY_ANS;

        memcpy(reserved_message[VERIFY_FALSE].begin(), (void*)&header, message_header_size);
        verify_message.SerializeToArray(reserved_message[VERIFY_FALSE].begin() + message_header_size, header.size);

        reserved_message_size[VERIFY_FALSE] = message_header_size + header.size;
    }

    // LOGOUT TRUE
    // -------------------------------------------------------------
    {
        chat_server::packet_logout_ans logout_message;
        logout_message.set_is_successful(true);

        MESSAGE_HEADER header;

        header.size = logout_message.ByteSize();
        header.type = chat_server::LOGOUT_ANS;

        memcpy(reserved_message[LOGOUT_TRUE].begin(), (void*)&header, message_header_size);
        logout_message.SerializeToArray(reserved_message[LOGOUT_TRUE].begin() + message_header_size, header.size);

        reserved_message_size[LOGOUT_TRUE] = message_header_size + header.size;
    }

    // LOGOUT FALSE
    // -------------------------------------------------------------
    {
        chat_server::packet_logout_ans logout_message;
        logout_message.set_is_successful(false);

        MESSAGE_HEADER header;

        header.size = logout_message.ByteSize();
        header.type = chat_server::LOGOUT_ANS;

        memcpy(reserved_message[LOGOUT_FALSE].begin(), (void*)&header, message_header_size);
        logout_message.SerializeToArray(reserved_message[LOGOUT_FALSE].begin() + message_header_size, header.size);

        reserved_message_size[LOGOUT_FALSE] = message_header_size + header.size;
    }

    // WHISPER ERROR
    // -------------------------------------------------------------
    {
        chat_server::packet_chat_whisper error_message;
        error_message.set_user_id("Error");
        error_message.set_target_id("");
        error_message.set_chat_message("");

        MESSAGE_HEADER header;

        header.size = error_message.ByteSize();
        header.type = chat_server::WHISPER;

        memcpy(reserved_message[WHISPER_ERROR].begin(), (void*)&header, message_header_size);
        error_message.SerializeToArray(reserved_message[WHISPER_ERROR].begin() + message_header_size, header.size);

        reserved_message_size[WHISPER_ERROR] = message_header_size + header.size;
    }
    // -------------------------------------------------------------

    for (int i = 0; i < 5; i++)
    {
        if (reserved_message_size[i] == 0 || reserved_message[i].data() == nullptr)
            return false;
    }

    return true;
}

void tcp_server::close_session(const int session_id)
{
    std::string user_key = session_list_[session_id]->get_user_key();
    std::string user_id = session_list_[session_id]->get_user_id();
    
    if (connected_session_map_.find(user_id) != connected_session_map_.end())
        connected_session_map_.erase(user_id);

    LOG_INFO << "Client connection closed. session_id: " << session_id;


    redis_connector::get_instance()->del(user_key);

    LOG_INFO << "Redis key deleted. user_key: " << user_key << " / user_id: " << user_id;


    session_list_[session_id]->get_socket().close();
    session_queue_.push_back(session_id);

    if (is_accepting_ == false)
        post_accept();
}

void tcp_server::process_packet(const int session_id, const int size, BYTE* packet)
{
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)packet;
    
    switch (message_header->type)
    {
    case chat_server::VERIFY_REQ:
        {
            chat_server::packet_verify_req verify_message;
            verify_message.ParseFromArray(packet + message_header_size, message_header->size);
        
            std::string redis_key = verify_message.key_string();
            std::string redis_value = verify_message.value_user_id();

            // 레디스 인증 키 일치
            if (redis_connector::get_instance()->get(redis_key) == redis_value)
            {
                if (!session_list_[session_id]->set_user_key(redis_key) || !session_list_[session_id]->set_user_id(redis_value) || !session_list_[session_id]->set_status(lobby))
                    LOG_ERROR << "process_packet() - User info setting failed.";

                connected_session_map_.insert(std::pair<std::string, tcp_session*>(redis_value, session_list_[session_id]));

                LOG_INFO << "Client cookie verified. user_id: " << session_list_[session_id]->get_user_id();
                session_list_[session_id]->post_send(false, reserved_message_size[VERIFY_TRUE], reserved_message[VERIFY_TRUE].begin());


                // 입장 메세지
                chat_server::packet_chat_normal normal_message;
                normal_message.set_user_id("SYSTEM");
                normal_message.set_chat_message("<" + redis_value + "> entered \nthe lobby.");

                MESSAGE_HEADER header;

                header.size = normal_message.ByteSize();
                header.type = chat_server::NORMAL;

                boost::array<BYTE, 1024> send_buffer;

                memcpy(send_buffer.begin(), (void*)&header, message_header_size);
                normal_message.SerializeToArray(send_buffer.begin() + message_header_size, header.size);

                master_data_queue_.push_back(send_buffer);

                LOG_INFO << "[CHAT_system] " << "<" + redis_value + "> entered the lobby.";

                for (auto iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
                {
                    if (iter->second->get_socket().is_open() && iter->second->get_status() == lobby)
                        iter->second->post_send(false, message_header_size + header.size, master_data_queue_.back().begin());
                }
                
            }
            // 불일치
            else
            {
                LOG_WARN << "process_packet() - Client cookie does not verified. user_id: " << session_list_[session_id]->get_user_id();
                session_list_[session_id]->post_send(false, reserved_message_size[VERIFY_FALSE], reserved_message[VERIFY_FALSE].begin());
            }

        }
        break;

    //case chat_server::LOGOUT_REQ:
    //    {
    //        chat_server::packet_logout_req logout_message;
    //        logout_message.ParseFromArray(packet + message_header_size, message_header->size);

    //        std::string user_id = logout_message.user_id();

    //        // 의미없는 if문
    //        if (user_id == session_list_[session_id]->get_user_id())
    //        {
    //            LOG_INFO << "Client logout successed. user_id: " << session_list_[session_id]->get_user_id();
    //            session_list_[session_id]->post_send(false, reserved_message_size[LOGOUT_TRUE], reserved_message[LOGOUT_TRUE].begin());
    //        }
    //        else
    //        {
    //            LOG_WARN << "process_packet() - Client logout failed. user_id: " << session_list_[session_id]->get_user_id();
    //            session_list_[session_id]->post_send(false, reserved_message_size[LOGOUT_FALSE], reserved_message[LOGOUT_FALSE].begin());
    //        }

    //    }
    //    break;


    case chat_server::ENTER_MATCH_NTF:
        {
            chat_server::packet_enter_match_ntf enter_match_message;
            enter_match_message.ParseFromArray(packet + message_header_size, message_header->size);

            std::string opponent_id = enter_match_message.opponent_id();

            auto iter = connected_session_map_.find(opponent_id);
            session_list_[session_id]->set_opponent_session(iter->second);
            session_list_[session_id]->set_status(room);

            LOG_INFO << "<" + session_list_[session_id]->get_user_id() + "> <" + opponent_id + "> entered the room.";
        }
        break;

    case chat_server::LEAVE_MATCH_NTF:
        {
            session_list_[session_id]->set_opponent_session(nullptr);
            session_list_[session_id]->set_status(lobby);

            LOG_INFO << "<" + session_list_[session_id]->get_user_id() + "> left the room.";
        }
        break;


    case chat_server::NORMAL:
        {
            chat_server::packet_chat_normal normal_message;
            normal_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            memcpy(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            LOG_INFO << "[CHAT_normal] " << normal_message.user_id() << ": " << normal_message.chat_message();
            
            for (auto iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
            {
                if (iter->second->get_socket().is_open() && iter->second->get_status() == lobby)
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;

    case chat_server::WHISPER:
        {
            chat_server::packet_chat_whisper whisper_message;
            whisper_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            memcpy(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            auto iter = connected_session_map_.find(whisper_message.target_id());
            if (session_list_[session_id]->get_socket().is_open() && iter != connected_session_map_.end())
            {
                // 본인에게 귓속말을 한 경우
                if (session_list_[session_id]->get_user_id() == iter->second->get_user_id())
                {
                    master_data_queue_.pop_back();

                    LOG_INFO << "[CHAT_whisper_error] " << whisper_message.user_id() << "->" << whisper_message.target_id() << ": " << whisper_message.chat_message();
                    
                    session_list_[session_id]->post_send(false, reserved_message_size[WHISPER_ERROR], reserved_message[WHISPER_ERROR].begin());
                }
                // 정상적인 경우
                else
                {
                    LOG_INFO << "[CHAT_whisper] " << whisper_message.user_id() << "->" << whisper_message.target_id() << ": " << whisper_message.chat_message();

                    session_list_[session_id]->post_send(false, size, master_data_queue_.back().begin());
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
                }
            }
            // 귓속말 상대가 없는 경우
            else
            {
                master_data_queue_.pop_back();

                LOG_INFO << "[CHAT_whisper_error] " << whisper_message.user_id() << "->" << whisper_message.target_id() << ": " << whisper_message.chat_message();

                session_list_[session_id]->post_send(false, reserved_message_size[WHISPER_ERROR], reserved_message[WHISPER_ERROR].begin());
            }
        }
        break;

    case chat_server::ROOM:
        {
            chat_server::packet_chat_room room_message;
            room_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            memcpy(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            LOG_INFO << "[CHAT_room] " << room_message.user_id() << "->" << session_list_[session_id]->get_opponent_session()->get_user_id() << ": " << room_message.chat_message();

            if (session_list_[session_id]->get_socket().is_open() && session_list_[session_id]->get_status() == room)
            {
                session_list_[session_id]->post_send(false, size, master_data_queue_.back().begin());
                session_list_[session_id]->get_opponent_session()->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;
    
    case chat_server::NOTICE:
        {
            chat_server::packet_chat_notice notice_message;
            notice_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            memcpy(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            LOG_INFO << "[CHAT_notice] " << notice_message.user_id() << ": " << notice_message.chat_message();

            for (auto iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
            {
                if (iter->second->get_socket().is_open())
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;


    // 패킷 타입 불명
    default:
        LOG_WARN << "process_packet() - Message type is unknown. : " << message_header->type;
        break;
    }
}


// ---------- private ----------
void tcp_server::post_accept()
{
   if (session_queue_.empty())
    {
        is_accepting_ = false;
        
        return;
    }

    is_accepting_ = true;
    int session_id = session_queue_.front();

    session_queue_.pop_front();

    acceptor_.async_accept(session_list_[session_id]->get_socket(),
        boost::bind(&tcp_server::handle_accept, this,
            session_list_[session_id],
            boost::asio::placeholders::error)
    );
}

void tcp_server::handle_accept(tcp_session* session, const boost::system::error_code& error)
{
    if (!error)
    {
        LOG_INFO << "Client connection successed. session_id: " << session->get_session_id();
        
        session->post_receive();
        io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    }
    else
    {
        // UTF-8 변환
        std::string err_msg = CW2A(CA2W(error.message().c_str()), CP_UTF8);

        LOG_WARN << "handle_accept() - Error Message: " << err_msg;
        io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    }
}
