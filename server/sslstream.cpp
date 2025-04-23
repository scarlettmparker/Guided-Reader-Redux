#include "sslstream.hpp"

namespace sslstream
{
  std::shared_ptr<ssl::stream<tcp::socket>> SSLStreamWrapper::current_stream_;

  void SSLStreamWrapper::set_current_stream(std::shared_ptr<ssl::stream<tcp::socket>> stream)
  {
    current_stream_ = stream;
  }

  std::shared_ptr<ssl::stream<tcp::socket>> SSLStreamWrapper::get_current_stream()
  {
    return current_stream_;
  }
}