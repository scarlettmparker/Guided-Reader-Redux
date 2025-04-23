#include "api.hpp"

using namespace postgres;
class PolicyHandler : public RequestHandler
{
private:
  ConnectionPool &pool;

  /**
   * Select the accepted policy field for a specific user.
   * Used to check if the user has already accepted the policy when trying to
   * accept it.
   *
   * @param user_id ID of the user to select the policy for.
   * @param verbose Whether to print messages to stdout.
   * @return true if the policy was accepted, false otherwise.
   */
  bool select_accepted_policy(int user_id, bool verbose)
  {
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
        verbose &&std::cerr << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty() || r[0][0].is_null())
      {
        verbose &&std::cout << "Policy not found" << std::endl;
        return false;
      }
      return r[0][0].as<bool>();
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
    }
    return false;
  }

  /**
   * Set the accepted policy field for a specific user.
   * Will either accept or reject the policy. Probably won't need to reject
   * policies ever (unless they change), but it's useful to have.
   *
   * @param user_id ID of the user to set the policy for.
   * @param accepted Whether the policy was accepted or not.
   * @param verbose Whether to print messages to stdout.
   * @return true if the policy was set, false otherwise.
   */
  bool set_accepted_policy(int user_id, bool accepted, bool verbose)
  {
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
        verbose &&std::cerr << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.affected_rows() == 0)
      {
        verbose &&std::cout << "Accepted policy field not found" << std::endl;
        return false;
      }
      return true;
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
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
    if (middleware::rate_limited(ip_address, "/policy", 1))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::post)
    {
      /**
       * POST to accept the privacy policy.
       */
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

      // Ensure the user is authenticated
      std::string_view session_id = request::get_session_id_from_cookie(req);
      if (session_id.empty())
      {
        return request::make_unauthorized_response("Session ID not found", req);
      }
      if (!request::validate_session(std::string(session_id), false))
      {
        return request::make_unauthorized_response("Invalid session ID", req);
      }

      int real_user_id = request::get_user_id_from_session(std::string(session_id), false);
      if (real_user_id == -1)
      {
        return request::make_unauthorized_response("Invalid user ID", req);
      }
      if (real_user_id != user_id)
      {
        return request::make_unauthorized_response("User ID mismatch", req);
      }

      // Check if user has already accepted the policy
      bool accepted_policy = select_accepted_policy(user_id, true);
      if (accepted_policy)
      {
        return request::make_bad_request_response("Policy already accepted", req);
      }
      if (!set_accepted_policy(user_id, true, true))
      {
        // TODO: add an internal server error response
        return request::make_bad_request_response("Failed to accept policy", req);
      }
      return request::make_ok_request_response("Policy accepted", req);
    }
    else
    {
      return request::make_bad_request_response("Invalid method", req);
    }
  }
};

extern "C" RequestHandler *create_policy_handler()
{
  return new PolicyHandler(get_connection_pool());
}