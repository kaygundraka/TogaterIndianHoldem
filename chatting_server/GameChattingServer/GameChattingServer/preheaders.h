#pragma once


// standard
#include <iostream>
#include <fstream>
#include <string>
#include <locale>

// windows
#include <atlstr.h>

// boost::log
#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>

// boost
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/locale.hpp>
#include <boost/array.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/unordered_map.hpp>
#include <boost/circular_buffer.hpp>

// json spirit
#include <json_spirit.h>

// protocol buffer
#include "chat_protobuf.pb.h"

// redispp
#include "redispp.h"
