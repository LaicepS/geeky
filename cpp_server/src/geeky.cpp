#include <boost/asio/io_context.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast.hpp>

using namespace std;
using namespace boost::asio::ip;

// server::server(const std::string &address, const std::string &port,
//                const std::string &doc_root, std::size_t thread_pool_size)
//     : thread_pool_size_(thread_pool_size), acceptor_(io_service_),
//       new_connection_(new connection(io_service_, request_handler_)),
//       request_handler_(doc_root) {
//   // Open the acceptor with the option to reuse the address (i.e.
//   SO_REUSEADDR).
//   boost::asio::ip::tcp::resolver resolver(io_service_);
//   boost::asio::ip::tcp::resolver::query query(address, port);
//   boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
//   acceptor_.open(endpoint.protocol());
//   acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
//   acceptor_.bind(endpoint);
//   acceptor_.listen();
//   acceptor_.async_accept(new_connection_->socket(),
//                          boost::bind(&server::handle_accept, this,
//                                      boost::asio::placeholders::error));
// }
//
// void server::run() {
//   // Create a pool of threads to run all of the io_services.
//   std::vector<boost::shared_ptr<boost::thread>> threads;
//   for (std::size_t i = 0; i < thread_pool_size_; ++i) {
//     boost::shared_ptr<boost::thread> thread(new boost::thread(
//         boost::bind(&boost::asio::io_service::run, &io_service_)));
//     threads.push_back(thread);
//   }
//
//   // Wait for all threads in the pool to exit.
//   for (std::size_t i = 0; i < threads.size(); ++i)
//     threads[i]->join();
// }
//
// void server::stop() { io_service_.stop(); }
//
// void server::handle_accept(const boost::system::error_code &e) {
//   if (!e) {
//     new_connection_->start();
//     new_connection_.reset(new connection(io_service_, request_handler_));
//     acceptor_.async_accept(new_connection_->socket(),
//                            boost::bind(&server::handle_accept, this,
//                                        boost::asio::placeholders::error));
//   }
// }

///// The top-level class of the HTTP server.
// class server
//  : private boost::noncopyable
//{
// public:
//  /// Construct the server to listen on the specified TCP address and port,
//  and
//  /// serve up files from the given directory.
//  explicit server(const std::string& address, const std::string& port,
//      const std::string& doc_root, std::size_t thread_pool_size);
//
//  /// Run the server's io_service loop.
//  void run();
//
//  /// Stop the server.
//  void stop();
//
// private:
//  /// Handle completion of an asynchronous accept operation.
//  void handle_accept(const boost::system::error_code& e);
//
//  /// The number of threads that will call io_service::run().
//  std::size_t thread_pool_size_;
//
//  /// The io_service used to perform asynchronous operations.
//  boost::asio::io_service io_service_;
//
//  /// Acceptor used to listen for incoming connections.
//  boost::asio::ip::tcp::acceptor acceptor_;
//
//  /// The next connection to be accepted.
//  connection_ptr new_connection_;
//
//  /// The handler for all incoming requests.
//  request_handler request_handler_;
//};

#include <utility>
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

struct http_server {

  http_server(unsigned short port) : io_context_{1}, socket_(io_context_) {

    auto const endpoint = tcp::endpoint(tcp::v4(), port);
    auto connection_acceptor = setup_acceptor(endpoint);

    while (true) {
      auto socket = tcp::socket{io_context_};

      connection_acceptor.accept(socket);

      thread(bind(http_session, std::move(socket))).detach();
    }
  }

  tcp::acceptor setup_acceptor(tcp::endpoint const &endpoint) {
    tcp::acceptor acceptor{io_context_};
    acceptor.open(endpoint.protocol());
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();

    return acceptor;
  }

  // boost::asio::http_server server;
  /// Acceptor used to listen for incoming connections.

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

void telnet_test() {
  auto const port = 8081;

  std::thread t([=]() { http_server server(port); });

  std::this_thread::sleep_for(10ms);

  connect(port);

  std::this_thread::sleep_for(10ms);
  connect(port);

  std::this_thread::sleep_for(10ms);

  t.detach();
};

void run_tests() { telnet_test(); }

int main() {
  run_tests();
  return 0;
}
