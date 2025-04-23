#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

namespace httpclient
{
  class HTTPClient
  {
  public:
    HTTPClient(const std::string &host, const std::string &port, bool use_ssl = true);

    // Http request methods
    std::string get(const std::string &target);
    std::string post(const std::string &target, const std::string &body);
    std::string put(const std::string &target, const std::string &body);
    std::string patch(const std::string &target, const std::string &body);
    std::string delete_(const std::string &target);

    void set_content_type(const std::string &content_type);
    void set_authorization(const std::string &auth_header);

  private:
    std::string do_request(
        beast::http::verb method, const std::string &target, const std::string &body = "");

    std::string auth_header_;
    std::string content_type_;
    std::string host_;
    std::string port_;
    bool use_ssl_;
    boost::asio::io_context ioc_;
    boost::asio::ssl::context ctx_{boost::asio::ssl::context::tlsv12_client};
  };
}

#endif