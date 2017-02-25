#pragma once
/* STL */
#include <iostream>
#include <unordered_map>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <fstream>
/* Google Protocol Buffers */
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include "channel_protobuf.pb.h"
/* boost lib */
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
/* singleton */
#include "singleton.h"
/* my sql */
#include <mysql.h>
/* redis */
#include "redispp.h"
/* json spirit -> config */
#include <json_spirit.h>
/* spd log */
#include <spdlog\spdlog.h>

using namespace std;
using namespace google;
using namespace channel_server;

/* session status */
typedef enum session_status  
{
    WAIT
    , CONN
    , LOGIN
    , LOGOUT
    , MATCH_REQUEST
    , MATCH_APPLY
    , MATCH_RECVER
    , MATCH_COMPLETE
    , MATCH_CONFIRM
} status;

/* protobuf struct */
#define MAX_RATING 8
#define shared_que 7
/* packet about join */
typedef packet_join_req join_request;
typedef packet_join_ans join_response;

/* packet about friends */
typedef packet_friends_req friends_request;
typedef packet_friends_ans friends_response;

/* packet about game */
typedef packet_play_rank_game_req match_request;
typedef packet_play_rank_game_ans match_response;
typedef packet_matching_complete_ans match_complete;
typedef packet_matching_confirm match_confirm;
typedef packet_play_friends_game_rel match_with_friends_relay;

/* packet about logout*/
typedef packet_logout_req logout_request;
typedef packet_logout_ans logout_response;

/* packet about error */
typedef packet_error_message error_report;

struct packet_header
{
    protobuf::uint32 size;
    message_type type;
};

const int packet_header_size = sizeof(packet_header);

class packet_handler
{
public:
    char* incode_message(const friends_request& message)            { return incoding(message_type::FRIENDS_REQ, message); }
    char* incode_message(const friends_response& message)           { return incoding(message_type::FRIENDS_ANS, message); }
    char* incode_message(const match_with_friends_relay& message)   { return incoding(message_type::PLAY_FRIENDS_REL, message); }
    char* incode_message(const match_request& message)              { return incoding(message_type::PLAY_RANK_REQ, message); }
    char* incode_message(const match_response& message)             { return incoding(message_type::PLAY_RANK_ANS, message); }
    char* incode_message(const join_request& message)               { return incoding(message_type::JOIN_REQ, message); }
    char* incode_message(const join_response& message)              { return incoding(message_type::JOIN_ANS, message); }
    char* incode_message(const match_complete& message)             { return incoding(message_type::MATCH_COMPLETE, message); }
    char* incode_message(const match_confirm& message)              { return incoding(message_type::MATCH_CONFIRM, message); }
    char* incode_message(const error_report& message)               { return incoding(message_type::ERROR_MSG, message); }
    char* incode_message(const logout_request& message)             { return incoding(message_type::LOGOUT_REQ, message); }
    char* incode_message(const logout_response& message)            { return incoding(message_type::LOGOUT_ANS, message); }

    inline void decode_message(protobuf::Message &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }
private:
    char *incoding(const message_type type, const protobuf::Message& message)
    {
        char *incoding_data = new char[packet_header_size + message.ByteSize()];
        packet_header *header = (packet_header *)incoding_data;

        header->type = type;
        header->size = message.ByteSize();
        message.SerializePartialToArray(&incoding_data[packet_header_size], header->size);
        
        return incoding_data;
    }

    void PrintMessage(const protobuf::Message& message) const
    {
        string textFormatStr;
        protobuf::TextFormat::PrintToString(message, &textFormatStr);
        printf("%s\n", textFormatStr.c_str());
    }
};