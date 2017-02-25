#pragma once

#include "preheaders.h"


using namespace google;


typedef struct _MESSAGE_HEADER {
    protobuf::uint32 size;
    chat_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof(MESSAGE_HEADER);