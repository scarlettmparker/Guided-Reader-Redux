#include "request.hpp"

using namespace postgres;
namespace request
{
  thread_local std::shared_ptr<pqxx::connection> cached_connection;
  thread_local std::chrono::steady_clock::time_point last_used;
  thread_local std::unique_ptr<pqxx::work> current_txn;

  /**
   * Begin a transaction with the database.
   * This function will create a new connection if one does not exist or if the current connection is stale.
   * @param pool Connection pool to get a connection from.
   * @return Transaction object for the database.
   */
  pqxx::work &begin_transaction(postgres::ConnectionPool &pool)
  {
    static thread_local std::unique_ptr<pqxx::work> current_txn;

    std::shared_ptr<pqxx::connection> conn(pool.acquire(), [&pool](pqxx::connection *c)
                                           { pool.release(c); });

    current_txn = std::make_unique<pqxx::work>(*conn);
    return *current_txn;
  }

  /**
   * Get the session ID from a cookie in a request.
   * @param req Request to get the session ID from.
   * @return Session ID from the cookie.
   */
  std::string_view get_session_id_from_cookie(const http::request<http::string_body> &req)
  {
    auto cookie_iter = req.find(http::field::cookie);
    if (cookie_iter == req.end())
    {
      return {};
    }

    const boost::string_view cookie = cookie_iter->value();
    if (cookie.empty())
    {
      return {};
    }

    constexpr boost::string_view SESSION_KEY = "sessionId=";
    auto pos = cookie.find(SESSION_KEY);
    if (pos == std::string::npos)
    {
      return {};
    }

    pos += SESSION_KEY.length();
    auto end = cookie.find(';', pos);
    return std::string_view(cookie.data() + pos, end == std::string::npos ? cookie.length() - pos : end - pos);
  }

  /**
   * Get the user ID from a session ID.
   * @param session_id Session ID to get the user ID from.
   * @param verbose Whether to print messages to stdout.
   * @return User ID from the session ID.
   */
  int get_user_id_from_session(std::string session_id, bool verbose)
  {
    auto &redis = Redis::get_instance();
    std::string key = "session:" + session_id;

    try
    {
      auto val = redis.hget(key, "user_id");
      if (!val)
      {
        verbose &&std::cout << "Session ID " << session_id << " not found in Redis" << std::endl;
        return -1;
      }
      return std::stoi(*val);
    }
    catch (const sw::redis::Error &e)
    {
      verbose &&std::cout << "Error retrieving session data from Redis: " << e.what() << std::endl;
    }
    catch (const std::invalid_argument &e)
    {
      verbose &&std::cout << "Invalid user_id format in Redis: " << e.what() << std::endl;
    }
    catch (const std::out_of_range &e)
    {
      verbose &&std::cout << "User_id out of range in Redis: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while retrieving session data from Redis" << std::endl;
    }
    return -1;
  }

  /**
   * Split a session ID into the session ID and the signature.
   *
   * @param signed_session_id Session ID to split.
   * @param session_id Session ID extracted from the signed session ID.
   * @param signature Signature extracted from the signed session ID.
   * @return true if the session ID was split, false otherwise.
   */
  bool split_session_id(const std::string &signed_session_id, std::string &session_id, std::string &signature)
  {
    std::istringstream iss(signed_session_id);
    if (std::getline(iss, session_id, '.') && std::getline(iss, signature))
    {
      return true;
    }
    return false;
  }

  /**
   * Invalidate a session ID. This removes the session ID from Redis.
   * @param session_id Session ID to invalidate.
   * @param verbose Whether to print messages to stdout.
   * @return true if the session was invalidated, false otherwise.
   */
  bool invalidate_session(std::string session_id, bool verbose)
  {
    auto &redis = Redis::get_instance();
    std::string key = "session:" + session_id;

    try
    {
      if (!redis.exists(key))
      {
        verbose &&std::cout << "Session ID " << session_id << " not found" << std::endl;
        return false;
      }

      std::string user_id;
      try
      {
        user_id = redis.hget(key, "user_id").value();
      }
      catch (const sw::redis::Error &e)
      {
        verbose &&std::cout << "Error getting user_id for session " << session_id << ": " << e.what() << std::endl;
        return false;
      }

      std::string user_sessions_key = "user:" + user_id + ":sessions";
      if (!redis.srem(user_sessions_key, session_id))
      {
        verbose &&std::cout << "Failed to remove session ID from user sessions set" << std::endl;
        return false;
      }

      if (!redis.del(key))
      {
        verbose &&std::cout << "Failed to delete session ID " << session_id << std::endl;
        return false;
      }

      return true;
    }
    catch (const sw::redis::Error &e)
    {
      verbose &&std::cout << "Error deleting session ID " << session_id << ": " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while deleting session ID " << session_id << std::endl;
    }
    return false;
  }

  /**
   * Check if a session ID is valid. This checks if the session ID exists
   * and that the given user ID matches the user ID stored with the session ID.
   *
   * @param session_id Session ID to check.
   * @param user_id User ID to check.
   * @param verbose Whether to print messages to stdout.
   * @return true if the session is valid, false otherwise.
   */
  bool validate_session(std::string signed_session_id, bool verbose)
  {
    std::string session_id, signature;
    if (!split_session_id(signed_session_id, session_id, signature))
    {
      verbose &&std::cout << "Invalid session ID format" << std::endl;
      return false;
    }

    // Check if the session ID is signed
    std::string secret_key = READER_SECRET_KEY;
    std::string expected_signature = session::generate_hmac(session_id, secret_key);

    if (signature != expected_signature)
    {
      verbose &&std::cout << "Invalid session ID signature" << std::endl;
      return false;
    }

    auto &redis = Redis::get_instance();
    std::string key = "session:" + signed_session_id;

    if (!redis.exists(key))
    {
      verbose &&std::cout << "Session ID " << session_id << " not found" << std::endl;
      return false;
    }

    std::map<std::string, std::string> session_data;
    try
    {
      redis.hgetall(key, std::inserter(session_data, session_data.begin()));
    }
    catch (const sw::redis::Error &e)
    {
      verbose &&std::cout << "Redis error: " << e.what() << std::endl;
      return false;
    }

    if (session_data.empty())
    {
      verbose &&std::cout << "Session ID " << session_id << " not found" << std::endl;
      return false;
    }

    if (session_data.find("user_id") == session_data.end())
    {
      verbose &&std::cout << "Session ID " << session_id << " missing user ID" << std::endl;
      return false;
    }

    // Check if the session has expired
    if (session_data.find("expires_at") != session_data.end())
    {
      auto expires_at = std::stoll(session_data["expires_at"]);
      auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

      if (now > expires_at)
      {
        verbose &&std::cout << "Session ID " << session_id << " has expired" << std::endl;
        return false;
      }
    }

    return true;
  }

  /**
   * Parse a query string into a map of key-value pairs
   * This allows query strings to be extracted from URLS, to be used as parameters.
   *
   * @param query Query string to parse.
   * @return Map of key-value pairs from the query string.
   */
  std::map<std::string, std::string> parse_query_string(std::string_view query)
  {
    std::map<std::string, std::string> params;
    size_t start = 0;

    while (start < query.length())
    {
      size_t end = query.find('&', start);
      if (end == std::string::npos)
      {
        end = query.length();
      }

      size_t equals = query.find('=', start);
      if (equals != std::string::npos && equals < end)
      {
        std::string key = std::string(query.substr(start, equals - start));
        std::string value = std::string(query.substr(equals + 1, end - equals - 1));
        params[key] = value;
      }

      start = end + 1;
    }
    return params;
  }

  /**
   * Parse the given parameter from a request. This ensures that the parameter
   * parameter appears in the query string. If it does not, an empty optional is returned.
   *
   * @param req Request to parse the parameter from.
   * @param parameter Parameter to parse from the request.
   * @return Value of the parameter if it exists, empty optional otherwise.
   */
  std::optional<std::string> parse_from_request(const http::request<http::string_body> &req, const std::string &parameter)
  {
    std::string target_str(req.target());
    std::string_view target(target_str);
    auto query_pos = target.find('?');

    if (query_pos == std::string::npos)
    {
      // std::cout << "No query string found in URL" << std::endl;
      return std::nullopt;
    }

    auto query = target.substr(query_pos + 1);
    auto params = parse_query_string(std::string(query));

    if (params.find(parameter) == params.end())
    {
      // std::cout << "Missing category parameter" << std::endl;
      return std::nullopt;
    }

    return params[parameter];
  }

  /**
   * Create an unauthorized response with a given message.
   * @param message Message to include in the response.
   * @param req Request that caused the error.
   * @return Response with the given message.
   */
  http::response<http::string_body> make_unauthorized_response(
      const std::string &message, const http::request<http::string_body> &req)
  {

    http::response<http::string_body> res{http::status::unauthorized, req.version()};
    res.set(http::field::server, "Beast");
    res.set(http::field::content_type, "application/json");

    nlohmann::json error_response =
        {
            {"status", "error"},
            {"message", message}};

    res.body() = error_response.dump();
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
  }

  /**
   * Create a bad request response with a given message.
   * @param message Message to include in the response.
   * @param req Request that caused the error.
   * @return Response with the given message.
   */
  http::response<http::string_body> make_bad_request_response(
      const std::string &message, const http::request<http::string_body> &req)
  {

    http::response<http::string_body> res{http::status::bad_request, req.version()};
    res.set(http::field::server, "Beast");
    res.set(http::field::content_type, "application/json");

    nlohmann::json error_response =
        {
            {"status", "error"},
            {"message", message}};

    res.body() = error_response.dump();
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
  }

  /**
   * Create a too many requests response with a given message.
   * @param message Message to include in the response.
   * @param req Request that caused the error.
   * @return Response with the given message.
   */
  http::response<http::string_body> make_too_many_requests_response(
      const std::string &message, const http::request<http::string_body> &req)
  {

    http::response<http::string_body> res{http::status::too_many_requests, req.version()};
    res.set(http::field::server, "Beast");
    res.set(http::field::content_type, "application/json");

    nlohmann::json error_response =
        {
            {"status", "error"},
            {"message", message}};

    res.body() = error_response.dump();
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
  }

  /**
   * Create an OK request response with a given message.
   * @param message Message to include in the response.
   * @param req Request to send the response for.
   * @return Response with the given message.
   */
  http::response<http::string_body> make_ok_request_response(
      const std::string &message, const http::request<http::string_body> &req)
  {

    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "Beast");
    res.set(http::field::content_type, "application/json");

    nlohmann::json ok_response =
        {
            {"status", "ok"},
            {"message", message}};

    res.body() = ok_response.dump();
    res.keep_alive(req.keep_alive());
    res.prepare_payload();

    return res;
  }

  /**
   * Create a response with JSON information.
   * @param json_info JSON information to include in the response.
   * @param req Request to send the response for.
   * @return Response with the JSON information.
   */
  http::response<http::string_body> make_json_request_response(
      const nlohmann::json &json_info, const http::request<http::string_body> &req)
  {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "Beast");
    res.set(http::field::content_type, "application/json");

    nlohmann::json json_response =
        {
            {"status", "ok"},
            {"message", json_info}};

    res.body() = json_response.dump();
    res.keep_alive(req.keep_alive());
    res.prepare_payload();

    return res;
  }
}