#include <iostream>
#include <memory>

#include "boost/asio.hpp"
#include "boost/beast.hpp"

#include "file_map.h"
#include "http_session.h"

namespace http = boost::beast::http;

using tcp = boost::asio::ip::tcp;

namespace
{
using namespace gky;

auto make_html_response(std::string const& content)
{
  http::response<http::string_body> response;

  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.set(http::field::content_type, "text/html");

  response.body() = content;

  return response;
}

void fail(boost::beast::error_code ec, char const* what)
{
  std::cerr << what << ": " << ec.message() << "\n";
}

template <class Body, class Allocator, class Send>
auto handle_root_request(
    http::request<Body, http::basic_fields<Allocator>> const& req,
    Send const& send,
    file_map const& server_files)
{
  auto const content = server_files.at(std::string(req.target()));
  auto response = make_html_response(content);
  response.keep_alive(req.keep_alive());
  return send(std::move(response));
}

template <class Body, class Allocator, class Send>
auto handle_search_request(
    http::request<Body, http::basic_fields<Allocator>> const& req,
    Send const& send,
    file_map const& server_files)
{
  auto const content = server_files.at("/search.html");
  auto response = make_html_response(content);
  response.keep_alive(req.keep_alive());
  return send(std::move(response));
}

template <class Body, class Allocator, class Send>
void handle_request(
    http::request<Body, http::basic_fields<Allocator>>&& req,
    Send&& send,
    file_map const& server_files)
{
  // Returns a bad request response
  auto const bad_request = [&req](boost::beast::string_view why) {
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

  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != boost::beast::string_view::npos)
    return send(bad_request("Illegal request-target"));

  if (req.target().find("/search?") == 0) {
    return handle_search_request(req, send, server_files);
  } else if (req.target() == "/") {
    return handle_root_request(req, send, server_files);
  } else {
    return send(bad_request("Illegal request"));
  }
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
          boost::beast::bind_front_handler(
              &http_session_impl::on_write, self_.shared_from_this(),
              sp->need_eof()));
    }
  };

  http_session_impl(tcp::socket&& socket, file_map const& server_files)
      : stream_(std::move(socket)), lambda_(*this), server_files_(server_files)
  {
  }

  // Start the asynchronous operation
  void run()
  {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session.
    boost::asio::dispatch(
        stream_.get_executor(),
        boost::beast::bind_front_handler(
            &http_session_impl::do_read, shared_from_this()));
  }

  void do_read()
  {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    stream_.expires_after(std::chrono::seconds(30));

    http::async_read(
        stream_, buffer_, req_,
        boost::beast::bind_front_handler(
            &http_session_impl::on_read, shared_from_this()));
  }

  void on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
  {
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream)
      return do_close();

    if (ec)
      return fail(ec, "read");

    handle_request(std::move(req_), lambda_, server_files_);
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
        boost::beast::bind_front_handler(
            &http_session_impl::on_write, shared_from_this(), sp->need_eof()));
  }

  void on_write(
      bool close,
      boost::beast::error_code ec,
      std::size_t bytes_transferred)
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
    boost::beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
  }

  boost::beast::tcp_stream stream_;
  boost::beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  std::shared_ptr<void> res_;
  send_lambda lambda_;
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
