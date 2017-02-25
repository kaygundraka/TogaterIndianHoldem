#pragma once

// google protocol buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

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

// standard
#include <iostream>
#include <random>

// cpprest
#include <cpprest\http_client.h>
#include <cpprest\filestream.h>

// json Spirit
#include <json_spirit.h>

#pragma comment(lib, "libcpprest140_2_9")
//#pragma comment(lib, "libcpprest140d_2_9")
#pragma comment(lib, "zlibstatic")

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

using namespace boost::asio;
using namespace boost::asio::ip;

using namespace google;

using namespace google;
