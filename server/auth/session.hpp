#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <boost/beast/http.hpp>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include "../db/redis.hpp"

namespace http = boost::beast::http;

namespace session
{
  std::string generate_session_id();
  http::response<http::string_body> set_session_cookie(const std::string &signed_session_id);
  bool set_session_id(std::string signed_session_id, int user_id, int duration, std::string ip_address);
  std::string bytes_to_hex(const std::string &bytes);
  std::string generate_hmac(const std::string &data, const std::string &key);
}

#endif