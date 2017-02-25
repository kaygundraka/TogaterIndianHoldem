#include "pre_headers.h"
#include "tcp_server.h"
#include "log.h"
#include "logic_worker.h"

tcp_server::tcp_server(boost::asio::io_service& io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    wait_accept();

    connected_session_list_.reserve(5000);

    end = false;

    keep_arrive_thread = new std::thread(&tcp_server::check_connected_session, this);
}

void tcp_server::wait_accept()
{
    thread_sync sync;

    connected_session::pointer new_connection = connected_session::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->get_socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
        
    connected_session_list_.emplace_back(new_connection);
}

void tcp_server::handle_accept(connected_session::pointer new_connection, const boost::system::error_code& error)
{
    thread_sync sync;

    if (!error)
    {
        new_connection->start();
        wait_accept();
    }
    else
    {
        Log::WriteLog(_T("%s"), error.message().c_str());
    }
}

void tcp_server::end_server()
{
    thread_sync sync;

    end = true;

    keep_arrive_thread->join();
}

void tcp_server::check_connected_session()
{
    while (!end)
    {
        Sleep(5000);

        thread_sync sync;

        int leave_sessions = connected_session_list_.size();
        Log::WriteLog(_T("current_sessions : %d"), leave_sessions);
        //std::cout << "sessions : " << connected_session_list_.size() << std::endl;

        for (auto iter = connected_session_list_.begin(); iter != connected_session_list_.end();)
        {
            // 갑자기 끊긴 아이가 발생
            if (!(*iter)->get_socket().is_open() && (*iter)->accept_client())
            {
                if (!(*iter)->is_in_room())
                {
                    iter = connected_session_list_.erase(iter);
                    continue;
                }
                
                // 방이 생성되어 있는지를 확인
                if (logic_worker::get_instance()->is_create_room((*iter)->get_room_key()))
                    logic_worker::get_instance()->disconnect_room((*iter)->get_room_key(), (*iter)->get_player_key()); // 방을 삭제
                else // 생성되어 있지 않으면 룸키를 제거
                {
           w         (*iter)->set_room_state(false);
                    redis_connector::get_instance()->remove_room_info((*iter)->get_room_key());
                }

                iter++;
            }
            else
                iter++;
        }

        Log::WriteLog(_T("leave_sessions : %d"), leave_sessions - connected_session_list_.size());
    }
}