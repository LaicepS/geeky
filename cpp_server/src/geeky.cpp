#include <iostream>
#include <string>
#include <thread>
#include <utility>

#include <boost/asio/io_context.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/system/error_code.hpp>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast.hpp>

using namespace std;

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

using namespace boost::beast;

void http_session(tcp::socket &socket) {
  boost::system::error_code ec;
  auto buffer = beast::flat_buffer{};

  while (true) {
    auto http_request = http::request<http::string_body>{};
    http::read(socket, buffer, http_request, ec);
    if (ec == http::error::end_of_stream)
      return;

    // Respond to GET request
    http::response<http::string_body> a;
    http::write(socket, a, ec);
    break;
  }

  socket.shutdown(tcp::socket::shutdown_send);
}

using port = unsigned short;

struct http_server {

  http_server(port port) : io_context_{1}, socket_(io_context_) {

    auto connection_acceptor = setup_acceptor(port);

    while (true) {
      auto socket = tcp::socket{io_context_};

      connection_acceptor.accept(socket);

      thread(bind(http_session, std::move(socket))).detach();
    }
  }

  tcp::acceptor setup_acceptor(port port) {
    auto const endpoint = tcp::endpoint(tcp::v4(), port);

    tcp::acceptor acceptor{io_context_};
    acceptor.open(endpoint.protocol());
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();

    return acceptor;
  }

  boost::asio::io_context io_context_;
  tcp::socket socket_;
};

auto connect(unsigned short port) {
  auto ioc = io_context();

  auto stream = beast::tcp_stream{ioc};
  stream.connect(tcp::endpoint(tcp::v4(), port));

  auto const req = http::request<http::string_body>{http::verb::get, "/", 10};

  http::write(stream, req);

  auto read_buffer = beast::flat_buffer{};
  auto response = http::response<http::dynamic_body>{};
  http::response<http::dynamic_body> res;
  http::read(stream, read_buffer, res);
}

void http_test() {
  auto const port = 8081;

  std::thread t([=]() { http_server server(port); });

  std::this_thread::sleep_for(10ms);

  connect(port);

  std::this_thread::sleep_for(10ms);
  connect(port);

  std::this_thread::sleep_for(10ms);

  t.detach();
};

void run_tests() { http_test(); }

int main() {
  run_tests();
  return 0;
}
