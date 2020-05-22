#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

#include "http_helpers.h"

using namespace boost;
using namespace boost::beast;
using namespace boost::asio;
using namespace std;

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

#include <iostream>
namespace gky {

http_response http_request(http::verb method, port port, string const& path) {
  auto ioc = io_context();

  auto stream = beast::tcp_stream{ioc};
  stream.connect(tcp::endpoint(tcp::v4(), port));

  auto const version = 10;
  auto req = http::request<http::string_body>{method, path, version};

  http::write(stream, req);

  auto read_buffer = beast::flat_buffer{};
  auto response = http::response<http::dynamic_body>{};
  http::read(stream, read_buffer, response);
  auto const content = beast::buffers_to_string(response.body().cdata());

  // Gracefully close the socket
  beast::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  // not_connected happens sometimes
  // so don't bother reporting it.
  //
  if (ec && ec != beast::errc::not_connected)
    throw beast::system_error{ec};

  // If we get here then the connection is closed gracefully
  return {response, read_buffer};
}

http_response http_get(port port, const string& path) {
  return http_request(http::verb::get, port, path);
}
}  // namespace gky
