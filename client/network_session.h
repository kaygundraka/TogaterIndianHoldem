#ifndef __NETWORK_SESSION_H__
#define __NETWORK_SESSION_H__

// boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// google network_manager buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "my_thread_sync.h"

using boost::asio::ip::tcp;
using namespace google;

#define network_mgr network_manager::get_instance()

enum { BUFSIZE = 1024 };

class network_session : public multi_thread_sync<network_session> {
protected:
    tcp::socket* socket_;

    bool is_connected_;

    boost::array<unsigned char, BUFSIZE> recv_buf_;
    boost::array<unsigned char, BUFSIZE> send_buf_;

    boost::thread* work_thread_;
    boost::thread* recv_thread_;
    boost::asio::io_service io_service_;

public:
    network_session();
    ~network_session();

    bool is_run();

    void connect(std::string ip, std::string port);
    void disconnect();

    bool is_socket_open();

    virtual bool create() = 0;
    virtual bool destroy() = 0;

    virtual void handle_read() = 0;
};

#endif