#pragma once
#include "pre_headers.h"
#include "logic_server.pb.h"

using namespace google;

typedef struct _MESSAGE_HEADER {
    protobuf::uint32 size;
    logic_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof(MESSAGE_HEADER);

class connected_session : public boost::enable_shared_from_this<connected_session>
{
private:
    tcp::socket socket_;
    
    std::string room_key_;
    std::string player_key_;

    bool create_room_;
    bool enter_room_;
    bool is_accept_;

    bool is_remain;
    int pre_remain_size;

    boost::array<BYTE, BUFSIZE> remain_buf;
    boost::array<BYTE, BUFSIZE> packet_buf;
    boost::array<BYTE, BUFSIZE> temp_buf;

    boost::array<BYTE, BUFSIZE> recv_buf_;
    boost::array<BYTE, BUFSIZE> send_buf_;

    bool safe_disconnect_;
    
private:
    connected_session(boost::asio::io_service& io_service);

    void handle_read(const boost::system::error_code& error, size_t /*bytes_transferred*/);
    
    void process_packet_echo_ntf(logic_server::packet_echo_ntf packet);
    void process_packet_enter_req(logic_server::packet_enter_req packet);
    void process_packet_process_turn_ans(logic_server::packet_process_turn_ans packet);
    void process_packet_disconnect_room_ntf(logic_server::packet_disconnect_room_ntf packet);

    void porcess_packet_game_state_ntf(logic_server::packet_game_state_ntf packet);

public:
    void shut_down();
    
    bool is_connected();
    void set_safe_disconnect(bool safe_disconnect);
    bool is_safe_disconnect();

    std::string get_player_key();
    std::string get_room_key();

    bool is_in_room();
    void set_room_state(bool is_enter);
    bool accept_client();

    typedef boost::shared_ptr<connected_session> pointer;

    static pointer create(boost::asio::io_service& io_service);

    void handle_send(logic_server::message_type msg_type, const protobuf::Message& message);

    tcp::socket& get_socket();
    
    void start();
};