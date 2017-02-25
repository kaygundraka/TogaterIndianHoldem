#pragma once

// standard
#include <ctime>
#include <string>
#include <iostream>
#include <map>
#include <queue>
#include <fstream>
#include <vector>
#include <thread>
#include <random>
#include <tchar.h>

// boost
#include <boost/asio.hpp>
#include <boost/pool/pool.hpp>
#include <boost/function.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/config.hpp>
#include <boost/unordered_map.hpp>

// json Spirit
#include <json_spirit.h>

// speed Log
//#include <spdlog\spdlog.h>

// redispp
#include "redispp.h"

// google protocol buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

// mysql
#include <my_global.h>
//#include <WinSock2.h>
#include <mysql.h>

#include "logic_server.pb.h"

enum { BUFSIZE = 1024 };

using boost::asio::ip::tcp;

//namespace spd = spdlog;