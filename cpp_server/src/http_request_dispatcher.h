#pragma once

#include "boost/asio.hpp"
#include "boost/beast.hpp"

#include "file_map.h"
#include "test.h"

namespace gky
{
constexpr boost::string_view extract_search_items(
    boost::string_view search_request)
{
  auto const search_token = "/search?";
  return boost::string_view(search_request.substr(sizeof(search_token)));
}

unittest(test_search_items_extraction)
{
  assert("foo" == extract_search_items("/search?foo"));
  assert("/search?" == extract_search_items("/search?/search?"));
  assert("  bar\t/search?" == extract_search_items("/search?  bar\t/search?"));
}

template <class sender>
void dispatch_request(
    boost::beast::http::request<boost::beast::http::string_body>&& req,
    sender&& send,
    file_map const& server_files)
{
  // Returns a bad request response
  auto const bad_request = [&req](boost::beast::string_view why) {
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::bad_request, req.version()};
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  // Make sure we can handle the method
  if (req.method() != boost::beast::http::verb::get)
    return send(bad_request("Unknown HTTP-method"));

  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != boost::beast::string_view::npos)
    return send(bad_request("Illegal request-target"));

  if (req.target().find("/search?") == 0) {
    return handle_search_request(
        extract_search_items(req.target()), req.keep_alive(), send,
        server_files);
  } else if (req.target() == "/") {
    return handle_root_request(req, send, server_files);
  } else {
    return send(bad_request("Illegal request"));
  }
}

}  // namespace gky
