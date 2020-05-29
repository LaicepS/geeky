#pragma once

#include "boost/asio.hpp"

#include "file_map.h"

namespace gky {

struct http_session {
  virtual void run() = 0;

  virtual ~http_session() {}
};

std::shared_ptr<http_session> make_http_session(
    boost::asio::ip::tcp::socket&& socket,
    file_map const& server_files);
}

