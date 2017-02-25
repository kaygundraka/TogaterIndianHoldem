#include "pre_header.h"
#include "network_session.h"

network_session::network_session()
{
    is_connected_ = false;

    socket_ = nullptr;
    work_thread_ = nullptr;
}

network_session::~network_session() {}

bool network_session::is_run()
{
    thread_sync sync;

    return is_connected_;
}

void network_session::connect(std::string ip, std::string port)
{
    thread_sync sync;

    work_thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));

    tcp::resolver resolver(socket_->get_io_service());
    tcp::resolver::query query(tcp::v4(), ip, port);

    auto endPointIter = resolver.resolve(query);
    tcp::resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endPointIter != end)
    {
        socket_->close();
        socket_->connect(*endPointIter++, error);
    }

    if (error)
        throw boost::system::system_error(error);

    is_connected_ = true;

    recv_thread_ = new boost::thread(boost::bind(&network_session::handle_read, this));

    io_service_.run();
}

void network_session::disconnect()
{
    thread_sync sync;

    if (socket_->is_open())
    {
        socket_->shutdown(boost::asio::socket_base::shutdown_both);
        socket_->close();
    }
}

bool network_session::is_socket_open()
{
    thread_sync sync;

    if (socket_ == nullptr)
        return false;

    if (!socket_->is_open() && is_connected_)
    {
        is_connected_ = false;
        return false;
    }

    return true;
}