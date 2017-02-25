#ifndef __CHANNEL_SESSION_H__
#define __CHANNEL_SESSION_H__

#include "network_session.h"
#include "channel_protobuf.pb.h"

const std::string CHANNEL_SERVER_IP("192.168.1.203");
const std::string CHANNEL_SEFVER_PORT("8800");

class channel_session : public network_session {
private:
    typedef struct _MESSAGE_HEADER {
        protobuf::uint32 size;
        channel_server::message_type type;
    } MESSAGE_HEADER;

    const int message_header_size = sizeof(MESSAGE_HEADER);

    void handle_send(channel_server::message_type msg_type, const protobuf::Message& message);

    void process_packet_join_ans(channel_server::packet_join_ans packet);
    void process_packet_logout_ans(channel_server::packet_logout_ans packet);
    void process_packet_friend_ans(channel_server::packet_friends_ans packet);
    void process_packet_rank_game_ans(channel_server::packet_play_rank_game_ans packet);
    void process_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel packet);
    void process_packet_matching_complete_ans(channel_server::packet_matching_complete_ans packet);
    void process_packet_error_message(channel_server::packet_error_message packet);

    virtual void handle_read() override;

public:
    void send_packet_join_req(std::string key, std::string id);
    void send_packet_logut_req(bool flag);
    void send_packet_friend_req(channel_server::packet_friends_req::req_type type, channel_server::basic_info info);
    void send_packet_rank_game_req(bool flag);
    void send_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel::req_type type, std::string target_id);
    void send_packet_matching_confirm();

    virtual bool create() override;
    virtual bool destroy() override;
};

#endif