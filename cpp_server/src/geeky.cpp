#include <boost/asio/io_context.hpp>
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

using namespace boost::asio::ip;

using namespace boost::asio;

struct http_server;

struct http_session {
  http_session() {}
};

template <typename connection_broker> struct connection_handler {
  connection_handler(connection_broker *my_broker) : my_broker_(my_broker) {}

  void handle_connection(boost::system::error_code const &ec) {
    cout << "handle_connection" << endl;
    if (ec) {
      cout << "error?" << endl;
      cout << ec << endl;
      return;
    }

    http_session();

    my_broker_->use_handler(this);
  }

  connection_broker *my_broker_;
};

void handle_connection(tcp::socket &) { cout << "handle connection" << endl; }

struct http_server {

  using conn_handler = connection_handler<http_server>;

  http_server(unsigned short port)
      : io_context_{1}, socket_(io_context_), connection_handler_(this) {

    auto const endpoint = tcp::endpoint(tcp::v4(), port);
    auto connection_acceptor = setup_acceptor(endpoint);

    while (true) {
      auto socket = tcp::socket{io_context_};

      connection_acceptor.accept(socket);

      thread(bind(handle_connection, move(socket))).detach();
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
  conn_handler connection_handler_;
};

auto connect(string const &, unsigned short port) {
  boost::asio::io_context io_context;

  boost::asio::ip::tcp::endpoint endpoint(tcp::v4(), port);

  tcp::socket socket(io_context);
  socket.connect(endpoint);
  socket.close();
}

void telnet_test() {
  auto const port = 8081;
  auto const host = "localhost";
  std::thread t([=]() { http_server server(port); });

  std::this_thread::sleep_for(10ms);
  connect(host, port);

  std::this_thread::sleep_for(10ms);
  connect(host, port);

  std::this_thread::sleep_for(10ms);

  t.detach();
};

void run_tests() { telnet_test(); }

int main() {
  run_tests();
  return 0;
}
