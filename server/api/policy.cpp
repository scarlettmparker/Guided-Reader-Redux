#include "api.hpp"
#include "../utils.hpp"

using namespace postgres;
using namespace utils;

class PolicyHandler : public RequestHandler
{
private:
  ConnectionPool &pool;

  bool select_accepted_policy(int user_id)
  {
    Logger::instance().debug("Checking accepted policy for user_id=" + std::to_string(user_id));
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_accepted_policy",
          user_id);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        throw;
      }

      if (r.empty() || r[0][0].is_null())
      {
        return false;
      }
      return r[0][0].as<bool>();
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
    }
    return false;
  }

  bool set_accepted_policy(int user_id, bool accepted)
  {
    Logger::instance().debug("Setting accepted policy for user_id=" + std::to_string(user_id) + " to " + (accepted ? "true" : "false"));
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "set_accepted_policy",
          user_id, accepted);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        throw;
      }

      if (r.affected_rows() == 0)
      {
        return false;
      }
      return true;
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
    }
    return false;
  }

public:
  PolicyHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
  }

  std::string get_endpoint() const override
  {
    return "/policy";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    Logger::instance().info("Policy endpoint called: " + std::string(req.method_string()));
    if (middleware::rate_limited(ip_address, "/policy", 1))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::post)
    {
      Logger::instance().debug("POST policy accept requested");
      nlohmann::json json_request;
      try
      {
        json_request = nlohmann::json::parse(req.body());
      }
      catch (const nlohmann::json::parse_error &e)
      {
        return request::make_bad_request_response("Invalid JSON", req);
      }
      if (!json_request.contains("user_id"))
      {
        return request::make_bad_request_response("Missing parameters user_id", req);
      }

      int user_id;
      try
      {
        user_id = json_request["user_id"].get<int>();
      }
      catch (const nlohmann::json::type_error &e)
      {
        return request::make_bad_request_response("Invalid parameter types", req);
      }

      std::string_view session_id = request::get_session_id_from_cookie(req);
      if (session_id.empty())
      {
        return request::make_unauthorized_response("Session ID not found", req);
      }
      if (!request::validate_session(std::string(session_id)))
      {
        return request::make_unauthorized_response("Invalid session ID", req);
      }

      int real_user_id = request::get_user_id_from_session(std::string(session_id));
      if (real_user_id == -1)
      {
        return request::make_unauthorized_response("Invalid user ID", req);
      }
      if (real_user_id != user_id)
      {
        return request::make_unauthorized_response("User ID mismatch", req);
      }

      bool accepted_policy = select_accepted_policy(user_id);
      if (accepted_policy)
      {
        Logger::instance().info("Policy already accepted for user_id=" + std::to_string(user_id));
        return request::make_bad_request_response("Policy already accepted", req);
      }
      if (!set_accepted_policy(user_id, true))
      {
        Logger::instance().error("Failed to accept policy for user_id=" + std::to_string(user_id));
        return request::make_bad_request_response("Failed to accept policy", req);
      }
      Logger::instance().info("Policy accepted for user_id=" + std::to_string(user_id));
      return request::make_ok_request_response("Policy accepted", req);
    }
    else
    {
      Logger::instance().info("Invalid method for policy endpoint");
      return request::make_bad_request_response("Invalid method", req);
    }
  }
};

extern "C" RequestHandler *create_policy_handler()
{
  return new PolicyHandler(get_connection_pool());
}