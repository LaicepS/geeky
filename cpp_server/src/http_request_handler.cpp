#include "http_request_handler.h"

#include "boost/beast.hpp"

#include "test.h"

using namespace gky;

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

struct bad_request_handler : request_handler {
  bad_request_handler(
      boost::beast::http::request<boost::beast::http::string_body>&& req,
      std::string const& why)
      : req(std::move(req)), why(why)
  {
  }

  virtual boost::beast::http::response<boost::beast::http::string_body>
  response() override
  {
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::bad_request, req.version()};
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  }

  boost::beast::http::request<boost::beast::http::string_body> req;
  std::string why;
};

struct root_request_handler : request_handler {
  root_request_handler(
      boost::beast::http::request<boost::beast::http::string_body> const& req,
      gky::file_map const& server_files)
      : req(req), server_files(server_files)
  {
  }

  virtual boost::beast::http::response<boost::beast::http::string_body>
  response() override
  {
    auto const content = server_files.at(std::string(req.target()));
    boost::beast::http::response<boost::beast::http::string_body> response;
    response.keep_alive(req.keep_alive());
    response.set(boost::beast::http::field::content_type, "text/html");
    response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.body() = content;
    return response;
  }

  boost::beast::http::request<boost::beast::http::string_body> req;
  gky::file_map const& server_files;
};

struct search_request_handler : request_handler {
  search_request_handler(bool keep_alive) : keep_alive_(keep_alive) {}

  virtual boost::beast::http::response<boost::beast::http::string_body>
  response() override
  {
    boost::beast::http::response<boost::beast::http::string_body> response;
    response.keep_alive(keep_alive_);
    response.set(boost::beast::http::field::content_type, "text/html");
    response.body() = "";
    return response;
  }

  bool keep_alive_;
};

namespace gky
{
std::unique_ptr<request_handler> make_request_handler(
    boost::beast::http::request<boost::beast::http::string_body>&& req,
    gky::file_map const& server_files)
{
  // Make sure we can handle the method
  if (req.method() != boost::beast::http::verb::get)
    return std::make_unique<bad_request_handler>(
        std::move(req), "Unknown HTTP-method");

  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != boost::beast::string_view::npos)
    return std::make_unique<bad_request_handler>(
        std::move(req), "Illegal request-target");
  else if (req.target() == "/")
    return std::make_unique<root_request_handler>(req, server_files);
  else if (req.target().find("/search?") == 0)
    return std::make_unique<search_request_handler>(req.keep_alive());
  else
    return std::make_unique<bad_request_handler>(
        std::move(req), "Illegal request-target");
}
}  // namespace gky

unittest(test_bad_request_makes_bad_request_handler)
{
  assert(dynamic_cast<bad_request_handler*>(
      make_request_handler(
          boost::beast::http::request<boost::beast::http::string_body>(
              boost::beast::http::verb::get, "..", 11),
          {})
          .get()));
}

unittest(test_root_request_makes_root_request_handler)
{
  assert(dynamic_cast<root_request_handler*>(
      make_request_handler(
          boost::beast::http::request<boost::beast::http::string_body>(
              boost::beast::http::verb::get, "/", 11),
          {{"/", "foo"}})
          .get()));
}

unittest(test_search_request_makes_search_handler)
{
  assert(dynamic_cast<search_request_handler*>(
      make_request_handler(
          boost::beast::http::request<boost::beast::http::string_body>(
              boost::beast::http::verb::get, "/search?toto", 11),
          {{"/", "foo"}})
          .get()));
}
