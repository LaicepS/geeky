#pragma once

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include <string>

namespace gky {
using port = unsigned short;
using path = std::string;

struct http_response {
  boost::beast::http::response<boost::beast::http::dynamic_body> response;
  boost::beast::flat_buffer buffer;
};

http_response http_request(boost::beast::http::verb method,
                           gky::port port,
                           std::string const& path);

http_response http_get(port port, const std::string& path);

}  // namespace gky
