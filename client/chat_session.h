#ifndef __CHAT_SESSION_H__
#define __CHAT_SESSION_H__

#include "network_session.h"
#include "chat_protobuf.pb.h"

const std::string CHAT_SERVER_IP("192.168.1.202");
const std::string CHAT_SERVER_PORT("8700");

class chat_session : public network_session {
private:
    typedef struct _MESSAGE_HEADER {
        protobuf::uint32 size;
        chat_server::message_type type;
    } MESSAGE_HEADER;

    const int message_header_size = sizeof(MESSAGE_HEADER);

    void handle_send(chat_server::message_type msg_type, const protobuf::Message& message);

    void process_packet_verify_ans(chat_server::packet_verify_ans packet);
    void process_pacekt_logout_ans(chat_server::packet_logout_ans packet);
   
    void process_packet_chat_normal(chat_server::packet_chat_normal packet);
    void process_packet_chat_whisper(chat_server::packet_chat_whisper packet);
    void process_packet_chat_room(chat_server::packet_chat_room packet);
    void process_packet_chat_notice(chat_server::packet_chat_notice packet);

    virtual void handle_read() override;

public:
    void send_packet_verify_req(std::string player_key, std::string id);
    void send_packet_logout_req(std::string player_id);

    void send_packet_enter_match_ntf(std::string target_id);
    void send_packet_leave_match_ntf();

    void send_packet_chat_normal(std::string id, std::string message);
    void send_packet_chat_whisper(std::string id, std::string target_id, std::string message);
    void send_packet_chat_room(std::string id, std::string message);
    

    virtual bool create() override;
    virtual bool destroy() override;
};

#endif