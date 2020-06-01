#include <iostream>
#include <memory>

#include "boost/asio.hpp"
#include "boost/beast.hpp"

#include "connection_listener.h"

using namespace gky;
using namespace boost::asio;
using namespace boost::asio::ip;

inline void fail(boost::beast::error_code ec, char const* what)
{
  std::cerr << what << ": " << ec.message() << "\n";
}

namespace
{
void throw_error(const boost::system::error_code& ec, const std::string& error)
{
  boost::asio::detail::throw_error(ec, error.c_str());
}

struct connection_listener_impl
    : std::enable_shared_from_this<connection_listener_impl>
    , connection_listener
{
  connection_listener_impl(
      io_context& ioc,
      unsigned short port,
      file_map const& http_files)
      : ioc_(ioc), acceptor_(make_strand(ioc)), http_files_(http_files)
  {
    auto endpoint = tcp::endpoint(tcp::v4(), port);
    setup_acceptor(endpoint);
  }

  void setup_acceptor(ip::tcp::endpoint endpoint)
  {
    boost::beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    throw_error(ec, "open");

    acceptor_.set_option(socket_base::reuse_address(true), ec);
    throw_error(ec, "set_option");

    acceptor_.bind(endpoint, ec);
    throw_error(ec, "bind");

    acceptor_.listen(socket_base::max_listen_connections, ec);
    throw_error(ec, "listen");
  }

  virtual void run() override
  {
    do_accept();
  }

  virtual void stop() override
  {
    ioc_.stop();
  }

  void do_accept()
  {
    acceptor_.async_accept(
        make_strand(ioc_),
        boost::beast::bind_front_handler(
            &connection_listener_impl::on_accept, shared_from_this()));
  }

  void on_accept(boost::beast::error_code ec, ip::tcp::socket socket)
  {
    if (ec)
      fail(ec, "accept");
    else
      make_http_session(std::move(socket), http_files_)->run();

    do_accept();
  }

  io_context& ioc_;
  ip::tcp::acceptor acceptor_;
  file_map http_files_;
};

}  // namespace

namespace gky
{
std::shared_ptr<connection_listener> make_connection_listener(
    io_context& ioc,
    unsigned short port,
    file_map const& http_files)
{
  return std::make_shared<connection_listener_impl>(ioc, port, http_files);
}
}  // namespace gky
