#include "server_session.h"
#include "channel_server.h"

session::session(int session_id, boost::asio::io_service &io_service, tcp_server* p_channel_serv,const int token_size, const int max_buffer_len)
    : session_id_(session_id)
    , socket_(io_service)
    , channel_serv_(p_channel_serv)
    , match_timer_(io_service)
    , token_size_(token_size)
    , max_buffer_len_(max_buffer_len)
{
    stat_ = status::WAIT;
    token_ = new char[token_size_];
    receive_buffer_ = new char[max_buffer_len_];
    temp_buffer_ = new char[max_buffer_len_];
    packet_buffer_ = new char[max_buffer_len_ * 2];
}

session::~session()
{
    while (send_data_queue_.empty() == false)
    {
        delete[] send_data_queue_.front();
        send_data_queue_.pop_front();
    }
    delete[] receive_buffer_;
    delete[] temp_buffer_;
    delete[] packet_buffer_;
}

void session::init()
{
    packet_buffer_mark_ = 0;
    boost::asio::socket_base::keep_alive keep_alive_op(true);
    socket_.set_option(keep_alive_op);
    boost::asio::socket_base::linger time_wait_op(true, 0);
    socket_.set_option(time_wait_op);

    while (send_data_queue_.empty() == false)
    {
        delete[] send_data_queue_.front();
        send_data_queue_.pop_front();
    }

}

void session::wait_receive()
{
    socket_.async_read_some(
        boost::asio::buffer(receive_buffer_,max_buffer_len_),
        boost::bind(
            &session::handle_receive, 
            this, 
            boost::asio::placeholders::error, 
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void session::wait_send(const bool immediately, const int send_data_size, char * send_data)
{
    if (immediately == false)
    {
        send_data_queue_.push_back(send_data);
    }


    if (immediately == false && send_data_queue_.size() > 1)
    {
        return;
    }

    boost::asio::async_write(
        socket_, boost::asio::buffer(send_data, send_data_size),   
        boost::bind(
            &session::handle_write,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void session::rematch(const boost::system::error_code & error)
{
    if(stat_ == status::MATCH_REQUEST) channel_serv_->rematching_request(this);
}


void session::control_timer_rematch(int sec, bool is_set)
{
    if (is_set && (sec > 0))
    {
        match_timer_.expires_from_now(std::chrono::seconds(sec));
        match_timer_.async_wait(boost::bind(&session::rematch, this, boost::asio::placeholders::error));
    }
}

void session::handle_write(const boost::system::error_code & error, size_t bytes_transferred)
{
    delete[] send_data_queue_.front();
    send_data_queue_.pop_front();

    if (stat_ == status::LOGOUT) 
    {
        while (send_data_queue_.empty() == false)
        {
            delete[] send_data_queue_.front();
            send_data_queue_.pop_front();
        }
        return;
    }

    if (send_data_queue_.empty() == false)
    {
        char *p_data = send_data_queue_.front();
        packet_header *p_header = (packet_header *)p_data;
        wait_send(true, p_header->size + packet_header_size, p_data);
    }
}

void session::handle_receive(const boost::system::error_code & error, size_t bytes_transferred)
{
    if (error)
    {
        if (error == boost::asio::error::eof)
        {
            log_manager::get_instance()->get_logger()->info("[Client Socket Close] [Socket Info : -session_id {0:d}]",session_id_);
            channel_serv_->close_session(session_id_);
        }
        else if (error == boost::asio::error::connection_reset)
        {
            log_manager::get_instance()->get_logger()->info("[Connection terminate] [Socket Info : -session_id {0:d}]", session_id_);
            channel_serv_->close_session(session_id_);
        }
        else
        {
            log_manager::get_instance()->get_logger()->warn("[Recv Error No : {0:d}] [Error Message : {1:s}] [User ID : {2:s}]", error.value(), error.message(), get_user_id());
        }
    }
    else
    {
        memcpy(&packet_buffer_[packet_buffer_mark_], receive_buffer_, bytes_transferred);

        int n_packet_data = packet_buffer_mark_ + bytes_transferred;
        int n_read_data = 0;
        while (n_packet_data > 0) 
        {
            if (n_packet_data < packet_header_size)
            {
                break;
            }
                       
            packet_header *p_header = (packet_header *)&packet_buffer_[n_read_data];

            if (p_header->size + packet_header_size <= n_packet_data)
            {
                channel_serv_->process_packet(session_id_, &packet_buffer_[n_read_data]);
                n_packet_data -= (p_header->size + packet_header_size);
                n_read_data += (p_header->size + packet_header_size);
            }
            else
            {
                break;
            }
        }

        if (n_packet_data > 0)
        {      
            memset(&temp_buffer_[0], 0, max_buffer_len_);
            memcpy(&temp_buffer_[0], &packet_buffer_[n_read_data], n_packet_data);
            memcpy(&packet_buffer_[0], &temp_buffer_[0], n_packet_data);
        } 

        packet_buffer_mark_ = n_packet_data;
        wait_receive();
    }
}
