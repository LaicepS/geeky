#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "connection_listener.h"
#include "filesystem.h"
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

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
//

using byte = unsigned char;


// Report a failure
void fail(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

file_map populate(string const& root) {
  file_map sv;
  sv["/"] = load_file(root + "/index.html");
  sv["/index.css"] = load_file(root + "/index.css");
  sv["/search.html"] = load_file(root + "/search.html");
  return sv;
}

auto server_guard(port port, file_map const& file_map) {
  struct server_guard {
    server_guard(::port port, ::file_map const& file_map) {
      auto const endpoint = tcp::endpoint(tcp::v4(), port);
      server_ = std::make_shared<connection_listener>(ioc_, endpoint, file_map);

      server_thread = thread([server = this->server_, &ioc = this->ioc_]() {
        server->run();
        ioc.run();
      });
    }

    ~server_guard() {
      server_->stop();
      server_thread.join();
    }

    io_context ioc_{1};
    std::shared_ptr<connection_listener> server_;
    thread server_thread;
  };

  return server_guard(port, file_map);
}

void test_get_root() {
  auto const port = 8081;
  auto const file_map = populate(dirname(__FILE__) + "/html");
  auto const server_guard = ::server_guard(port, file_map);

  auto [response, buffer] = http_get(port, "/");
  assert(http::to_status_class(response.result()) ==
         http::status_class::successful);
  assert(response.at("Content-Type") == "text/html");
  assert(beast::buffers_to_string(response.body().data()) == file_map.at("/"));
}

void test_unsupported_verb() {
  auto const port = 8081;
  auto const file_map = populate(dirname(__FILE__) + "/html");
  auto server_guard = ::server_guard(port, file_map);
  auto [response, _] = http_request(http::verb::mkcalendar, port, "/");
  assert(http::to_status_class(response.result()) ==
         http::status_class::client_error);
}

void test_search() {
  auto const port = 8081;
  auto const file_map = populate(dirname(__FILE__) + "/html");
  auto const server_guard = ::server_guard(port, file_map);

  auto [response, _] = http_get(port, "/search?toto");
  assert(http::to_status_class(response.result()) ==
         http::status_class::successful);
}

void test_get_wrong_url() {
  auto const port = 8081;
  auto const file_map = populate(dirname(__FILE__) + "/html");
  auto const server_guard = ::server_guard(port, file_map);

  auto [response, _] = http_get(port, "/bad_url");
  assert(http::to_status_class(response.result()) ==
         http::status_class::client_error);
}

void http_tests() {
  test_get_root();
  test_get_wrong_url();
  test_search();
  test_unsupported_verb();
};

void run_tests() {
  http_tests();
}

auto get_args(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: http-server-async <port> <root_path>\n"
              << "Example:\n"
              << "    http-server-async 8080 1\n";
    exit(0);
  }

  return make_tuple(::port(std::atoi(argv[1])), string(argv[2]));
}

void start_ioc_threads(io_context& ioc, int num_threads) {
  vector<thread> v;
  for (auto i = 0; i < num_threads - 1; i++)
    v.emplace_back([&ioc] { ioc.run(); });
  ioc.run();
}

int main(int argc, char* argv[]) {
  run_tests();

  auto const [port, root_path] = get_args(argc, argv);

  auto const threads = 1;
  net::io_context ioc{threads};

  auto const file_map = populate(root_path);

  std::make_shared<gky::connection_listener>(
      ioc, tcp::endpoint{tcp::v4(), port}, file_map)
      ->run();

  start_ioc_threads(ioc, threads);

  return 0;
}
