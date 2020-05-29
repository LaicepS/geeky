#pragma once

#include <memory>
#include <string>

#include "boost/asio.hpp"

#include "file_map.h"
#include "http_session.h"

namespace gky {

void throw_error(const boost::system::error_code& ec,
                 const std::string& error) {
  boost::asio::detail::throw_error(ec, error.c_str());
}

struct connection_listener : std::enable_shared_from_this<connection_listener> {
  connection_listener(boost::asio::io_context& ioc,
                      boost::asio::ip::tcp::endpoint endpoint,
                      file_map const& http_files)
      : ioc_(ioc),
        acceptor_(boost::asio::make_strand(ioc)),
        http_files_(http_files) {
    setup_acceptor(endpoint);
  }

  void setup_acceptor(boost::asio::ip::tcp::endpoint endpoint) {
    boost::beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    gky::throw_error(ec, "open");

    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    gky::throw_error(ec, "set_option");

    acceptor_.bind(endpoint, ec);
    gky::throw_error(ec, "bind");

    acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
    gky::throw_error(ec, "listen");
  }

  void run() { do_accept(); }

  void stop() { ioc_.stop(); }

  void do_accept() {
    acceptor_.async_accept(
        boost::asio::make_strand(ioc_),
        boost::beast::bind_front_handler(&connection_listener::on_accept,
                                         shared_from_this()));
  }

  void on_accept(boost::beast::error_code ec,
                 boost::asio::ip::tcp::socket socket) {
    if (ec)
      fail(ec, "accept");
    else
      std::make_shared<http_session>(std::move(socket), http_files_)->run();

    do_accept();
  }

  boost::asio::io_context& ioc_;
  boost::asio::ip::tcp::acceptor acceptor_;
  file_map http_files_;
};

}  // namespace gky
