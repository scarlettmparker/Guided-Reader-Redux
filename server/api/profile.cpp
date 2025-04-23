#include "api.hpp"

using namespace postgres;
class ProfileHandler : public RequestHandler
{
private:
  ConnectionPool &pool;

  /**
   * Select profile data from the database. This will return the profile information
   * of a user given their user ID. Will probably be changed to take from a "Profile"
   * table in the future, but currently will just get some public fields from the user
   * that are stored in the "User" table, and not seen in the navbar (e.g. proficiency levels).
   *
   * @param user_id ID of the user to select profile data from.
   * @param verbose Whether to print messages to stdout.
   * @return JSON of profile data.
   */
  nlohmann::json select_profile_data(int user_id, bool verbose)
  {
    nlohmann::json profile_info = nlohmann::json::array();

    try
    {
      pqxx::work &txn = request::begin_transaction(pool);

      pqxx::result r = txn.exec_prepared(
          "select_profile_data",
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

      if (r.empty())
      {
        verbose &&std::cout << "Profile with ID " << user_id << " not found" << std::endl;
        return profile_info;
      }

      profile_info = nlohmann::json::parse(r[0][0].as<std::string>());
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
    }
    return profile_info;
  }

public:
  ProfileHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
  }

  std::string get_endpoint() const override
  {
    return "/profile";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    if (middleware::rate_limited(ip_address, "/profile", 20))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
      /**
       * GET profile information.
       */
      std::optional<std::string> user_id_param = request::parse_from_request(req, "user_id");

      if (!user_id_param)
      {
        return request::make_bad_request_response("Missing parameter user_id", req);
      }
      int user_id;
      try
      {
        user_id = std::stoi(user_id_param.value());
      }
      catch (const std::invalid_argument &)
      {
        return request::make_bad_request_response("Invalid numeric value for user_id", req);
      }
      catch (const std::out_of_range &)
      {
        return request::make_bad_request_response("Number out of range for user_id", req);
      }

      nlohmann::json profile_info = select_profile_data(user_id, false);
      if (profile_info.empty())
      {
        return request::make_bad_request_response("No profile found", req);
      }

      return request::make_json_request_response(profile_info, req);
    }
    else
    {
      return request::make_bad_request_response("Invalid request method", req);
    }
  }
};

extern "C" RequestHandler *create_profile_handler()
{
  return new ProfileHandler(get_connection_pool());
}