#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <iostream>
#include <string>
#include <thread>

#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio::ip;

struct http_server_t {
  ~http_server_t() {}
};

auto http_server(int) { return http_server_t(); }

auto connect(string const &host, unsigned short port) {
  boost::asio::io_service io_service;

  boost::asio::ip::tcp::endpoint endpoint(
      boost::asio::ip::address::from_string(host), port);

  tcp::socket socket(io_service);
  socket.async_connect(endpoint, [](const boost::system::error_code &) -> void {
    cout << "toto" << endl;
  });

  while (true) {
    std::this_thread::sleep_for(500ms);
  }
}

void run_tests() {
  auto const handle = http_server(8080);
  connect("localhost", 8080);
}

int main() {
  run_tests();
  return 0;
}
