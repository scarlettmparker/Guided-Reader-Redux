#ifndef APIKEY_HPP
#define APIKEY_HPP

#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <random>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/beast/http.hpp>

#include "config.h"
#include "../db/redis.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

namespace apikey
{
  struct APIKey
  {
    std::string key;
    int request_limit;
    int requests_last_24h;
    std::vector<std::string> permissions;
  };

  bool api_key_exists(const std::string &api_key);
  APIKey get_api_key_details(const std::string &api_key);
  int get_request_count(const std::string &api_key);
  bool increment_request_count(const std::string &api_key);

  std::string generate_api_key();
  bool insert_api_key(const std::string &api_key, int request_limit, const std::vector<std::string> &permissions);
  bool update_api_key(const std::string &api_key, int request_limit, const std::vector<std::string> &permissions);
  bool destroy_api_key(const std::string &api_key);
  bool verify_api_key(const http::request<http::string_body> &req);
}

#endif