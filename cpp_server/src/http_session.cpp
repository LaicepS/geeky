#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "boost/asio.hpp"
#include "boost/beast.hpp"

#include "file_map.h"
#include "filesystem.h"
#include "html.h"
#include "http_request_handler.h"
#include "http_session.h"
#include "test.h"

namespace http = boost::beast::http;
namespace asio = boost::asio;
namespace beast = boost::beast;

using tcp = boost::asio::ip::tcp;

using namespace std::chrono_literals;
using namespace std;
using namespace http;

namespace
{
using namespace gky;

void fail(beast::error_code ec, char const* what)
{
  std::cerr << what << ": " << ec.message() << "\n";
}

using token = string;
using tokens = vector<token>;
using context = string;
using search_database = vector<pair<token, context>>;

html make_search_html(tokens const&, ::search_database const&)
{
  return html();
}

unittest(test_make_search_html)
{
  assert(make_search_html({"hello", "world"}, {}).to_string() == "");
}

struct http_session_impl
    : std::enable_shared_from_this<http_session_impl>
    , gky::http_session {
  // This is the C++11 equivalent of a generic lambda.
  // The function object is used to send an HTTP message.
  struct send_lambda {
    http_session_impl& self_;

    explicit send_lambda(http_session_impl& self) : self_(self) {}

    template <bool isRequest, class Body, class Fields>
    void operator()(http::message<isRequest, Body, Fields>&& msg) const
    {
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
          beast::bind_front_handler(
              &http_session_impl::on_write, self_.shared_from_this(),
              sp->need_eof()));
    }
  };

  http_session_impl(tcp::socket&& socket, file_map const& server_files)
      : stream_(std::move(socket)), send_(*this), server_files_(server_files)
  {
  }

  // Start the asynchronous operation
  void run()
  {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session.
    asio::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(
            &http_session_impl::do_read, shared_from_this()));
  }

  void do_read()
  {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    stream_.expires_after(30s);

    http::async_read(
        stream_, buffer_, req_,
        beast::bind_front_handler(
            &http_session_impl::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred)
  {
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream)
      return do_close();

    if (ec)
      return fail(ec, "read");

    send_(make_request_handler(std::move(req_), server_files_)->response());
  }

  template <bool isRequest, class Body, class Fields>
  void message_writer(http::message<isRequest, Body, Fields>&& msg)
  {
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
        beast::bind_front_handler(
            &http_session_impl::on_write, shared_from_this(), sp->need_eof()));
  }

  void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
  {
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

  void do_close()
  {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
  }

  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  std::shared_ptr<void> res_;
  send_lambda send_;

  file_map server_files_;
};

}  // namespace

namespace gky
{
std::shared_ptr<http_session> make_http_session(
    tcp::socket&& socket,
    file_map const& server_files)
{
  return std::shared_ptr<http_session>(
      new http_session_impl(std::move(socket), server_files));
}

}  // namespace gky
