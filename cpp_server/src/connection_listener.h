#pragma once

#include <memory>
#include <string>

#include "boost/asio.hpp"

#include "file_map.h"
#include "http_session.h"

namespace gky {
struct connection_listener {
  virtual void run() = 0;
  virtual void stop() = 0;
  virtual ~connection_listener() {}
};

std::shared_ptr<connection_listener> make_connection_listener(
    boost::asio::io_context& ioc,
    unsigned short port,
    file_map const& http_files);

}  // namespace gky
