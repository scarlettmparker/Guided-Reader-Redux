#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <boost/beast/http.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <sstream>

#include <string>
#include <string_view>
#include <map>
#include <optional>
#include <iostream>
#include <chrono>
#include <unordered_map>

#include "../auth/session.hpp"
#include "../db/postgres.hpp"
#include "../db/redis.hpp"
#include "config.h"

namespace http = boost::beast::http;

namespace request
{
  pqxx::work &begin_transaction(postgres::ConnectionPool &pool);
  std::string_view get_session_id_from_cookie(const http::request<http::string_body> &req);
  int get_user_id_from_session(std::string session_id, bool verbose);

  bool split_session_id(const std::string &signed_session_id, std::string &session_id, std::string &signature);
  bool invalidate_session(std::string session_id, bool verbose);
  bool validate_session(std::string session_id, bool verbose);

  std::map<std::string, std::string> parse_query_string(std::string_view query);
  std::optional<std::string> parse_from_request(const http::request<http::string_body> &req, const std::string &parameter);

  /* request responses */
  http::response<http::string_body> make_unauthorized_response(const std::string &message, const http::request<http::string_body> &req);
  http::response<http::string_body> make_bad_request_response(const std::string &message, const http::request<http::string_body> &req);
  http::response<http::string_body> make_too_many_requests_response(const std::string &message, const http::request<http::string_body> &req);
  http::response<http::string_body> make_ok_request_response(const std::string &message, const http::request<http::string_body> &req);
  http::response<http::string_body> make_json_request_response(const nlohmann::json &json_info, const http::request<http::string_body> &req);
}
#endif