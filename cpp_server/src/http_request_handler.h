#pragma once

#include <memory>

#include "boost/beast.hpp"

#include "file_map.h"

namespace gky
{
struct request_handler
{
  virtual boost::beast::http::response<boost::beast::http::string_body>
  response() = 0;

  virtual ~request_handler() {}
};

std::unique_ptr<request_handler> make_request_handler(
    boost::beast::http::request<boost::beast::http::string_body>&& req,
    gky::file_map const& server_files);
}  // namespace gky
