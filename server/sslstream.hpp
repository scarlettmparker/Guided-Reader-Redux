#ifndef SSLSTREAM_HPP
#define SSLSTREAM_HPP

#include <memory>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace ssl = boost::asio::ssl;
using boost::asio::ip::tcp;

namespace sslstream
{
  class SSLStreamWrapper
  {
  private:
    static std::shared_ptr<ssl::stream<tcp::socket>> current_stream_;

  public:
    static void set_current_stream(std::shared_ptr<ssl::stream<tcp::socket>> stream);
    static std::shared_ptr<ssl::stream<tcp::socket>> get_current_stream();
  };
}

#endif