#include <c++/6/experimental/bits/fs_fwd.h>
#include <algorithm>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <cstddef>
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/system/error_code.hpp>

#include "http_helpers.h"

using namespace std;
using namespace gky;

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

using namespace boost::beast;

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

namespace fs = std::experimental::filesystem;

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
//

using byte = unsigned char;

using file_map = unordered_map<std::string, std::string>;

template <class Body, class Allocator, class Send>
void handle_request(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send,
                    ::file_map const& server_files) {
  // Returns a bad request response
  auto const bad_request = [&req](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  // Make sure we can handle the method
  if (req.method() != http::verb::get)
    return send(bad_request("Unknown HTTP-method"));

  // Request path must be absolute and not contain "..".
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return send(bad_request("Illegal request-target"));

  if (req.target() == "/search?") {
    http::response<http::dynamic_body> file_result;

    file_result.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    file_result.set(http::field::content_type, "text/html");
    file_result.keep_alive(req.keep_alive());

    beast::ostream(file_result.body()) << "<html></html>";

    return send(std::move(file_result));
  }

  // Respond to GET request
  auto const index_file = server_files.at(string(req.target()));

  http::response<http::dynamic_body> file_result;

  file_result.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  file_result.set(http::field::content_type, "text/html");
  file_result.keep_alive(req.keep_alive());

  beast::ostream(file_result.body()) << index_file;

  return send(std::move(file_result));
}

// Report a failure
void fail(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

struct session : std::enable_shared_from_this<session> {
  // This is the C++11 equivalent of a generic lambda.
  // The function object is used to send an HTTP message.
  struct send_lambda {
    session& self_;

    explicit send_lambda(session& self) : self_(self) {}

    template <bool isRequest, class Body, class Fields>
    void operator()(http::message<isRequest, Body, Fields>&& msg) const {
      // The lifetime of the message has to extend
      // for the duration of the async operation so
      // we use a shared_ptr to manage it.
      auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(
          std::move(msg));

      // Store a type-erased version of the shared
      // pointer in the class to keep it alive.
      self_.res_ = sp;

      // Write the response
      http::async_write(
          self_.stream_, *sp,
          beast::bind_front_handler(&session::on_write,
                                    self_.shared_from_this(), sp->need_eof()));
    }
  };

  session(tcp::socket&& socket, file_map const& server_files)
      : stream_(std::move(socket)),
        lambda_(*this),
        server_files_(server_files) {}

  // Start the asynchronous operation
  void run() {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&session::do_read, shared_from_this()));
  }

  void do_read() {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    stream_.expires_after(30s);

    http::async_read(
        stream_, buffer_, req_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream)
      return do_close();

    if (ec)
      return fail(ec, "read");

    handle_request(std::move(req_), lambda_, server_files_);
  }

  template <bool isRequest, class Body, class Fields>
  void message_writer(http::message<isRequest, Body, Fields>&& msg) {
    // The lifetime of the message has to extend
    // for the duration of the async operation so
    // we use a shared_ptr to manage it.
    auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(
        std::move(msg));

    // Store a type-erased version of the shared
    // pointer in the class to keep it alive.
    // self_.res_ = sp;

    // Write the response
    http::async_write(
        stream_, *sp,
        beast::bind_front_handler(&session::on_write, shared_from_this(),
                                  sp->need_eof()));
  }

  void on_write(bool close,
                beast::error_code ec,
                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    if (close) {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return do_close();
    }

    res_ = nullptr;

    do_read();
  }

  void do_close() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
  }

  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  std::shared_ptr<void> res_;
  send_lambda lambda_;
  file_map server_files_;
};

namespace gky {

void throw_error(const boost::system::error_code& ec, const string& error) {
  boost::asio::detail::throw_error(ec, error.c_str());
}

}  // namespace gky

struct listener : std::enable_shared_from_this<listener> {
  listener(net::io_context& ioc,
           tcp::endpoint endpoint,
           file_map const& server_files)
      : ioc_(ioc),
        acceptor_(net::make_strand(ioc)),
        server_files_(server_files) {
    setup_acceptor(endpoint);
  }

  void setup_acceptor(tcp::endpoint endpoint) {
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    gky::throw_error(ec, "open");

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    gky::throw_error(ec, "set_option");

    acceptor_.bind(endpoint, ec);
    gky::throw_error(ec, "bind");

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    gky::throw_error(ec, "listen");
  }

  void run() { do_accept(); }

  void stop() { ioc_.stop(); }

  void do_accept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec)
      fail(ec, "accept");
    else
      std::make_shared<session>(std::move(socket), server_files_)->run();

    do_accept();
  }

  net::io_context& ioc_;
  tcp::acceptor acceptor_;
  file_map server_files_;
};

#include <libgen.h>

string dirname(string const& path) {
  char c_path[128];
  strcpy(c_path, path.c_str());
  ::dirname(c_path);
  return c_path;
}

auto server_guard(port port) {
  struct server_guard {
    server_guard(::port port) {
      auto const endpoint = tcp::endpoint(tcp::v4(), port);
      auto const files = load_files();
      server_ = std::make_shared<listener>(ioc_, endpoint, files);

      server_thread = thread([server = this->server_, &ioc = this->ioc_]() {
        server->run();
        ioc.run();
      });
    }

    auto load_file(string const& path) {
      ifstream f(root_path + path);
      string content;
      string curr_line;
      while (std::getline(f, curr_line))
        content += curr_line;

      return content;
    }

    file_map load_files() {
      file_map sv;
      sv["/"] = load_file("/index.html");
      sv["/"] = load_file("/index.css");
      return sv;
    }

    ~server_guard() {
      server_->stop();
      server_thread.join();
    }

    io_context ioc_{1};
    std::shared_ptr<listener> server_;
    thread server_thread;
    const string root_path = dirname(__FILE__) + "/html";
  };

  return server_guard(port);
}

void test_get_root() {
  auto const port = 8081;
  auto const server_guard = ::server_guard(port);

  auto [response, buffer] = http_get(port, "/");
  assert(http::to_status_class(response.result()) ==
         http::status_class::successful);
  assert(response.at("Content-Type") == "text/html");
  assert(response.body().size());
}

void test_unsupported_verb() {
  auto const port = 8081;
  auto server_guard = ::server_guard(port);
  auto [response, _] = http_request(http::verb::mkcalendar, port, "/");
  assert(http::to_status_class(response.result()) ==
         http::status_class::client_error);
}

void test_search() {
  auto const port = 8081;
  auto const server_guard = ::server_guard(port);

  auto [response, _] = http_get(port, "/search?");
  assert(http::to_status_class(response.result()) ==
         http::status_class::successful);
}

void http_tests() {
  test_get_root();
  test_search();
  test_unsupported_verb();
};

void run_tests() {
  http_tests();
}

void check_args(int argc) {
  if (argc != 3) {
    std::cerr << "Usage: http-server-async <port> <root_path>\n"
              << "Example:\n"
              << "    http-server-async 8080 1\n";
    exit(0);
  }
}

int main(int argc, char* argv[]) {
  run_tests();

  check_args(argc);

  auto const port = static_cast<::port>(std::atoi(argv[1]));
  // auto const root_path = std::experimental::filesystem::path(argv[2]);
  auto const threads = 1;

  net::io_context ioc{threads};

  std::make_shared<listener>(ioc, tcp::endpoint{tcp::v4(), port}, file_map{})
      ->run();

  std::vector<std::thread> v;
  for (auto i = threads - 1; i > 0; --i)
    v.emplace_back([&ioc] { ioc.run(); });
  ioc.run();

  return 0;
}
