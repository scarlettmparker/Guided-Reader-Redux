#include "api.hpp"

using namespace postgres;
using namespace utils;

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
   * @return JSON of profile data.
   */
  nlohmann::json select_profile_data(int user_id)
  {
    Logger::instance().debug("Selecting profile data for user_id=" + std::to_string(user_id));
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
        utils::Logger::instance().error(std::string("Error committing transaction: ") + e.what());
        throw;
      }

      if (r.empty())
      {
        utils::Logger::instance().debug("Profile with ID " + std::to_string(user_id) + " not found");
        return profile_info;
      }

      profile_info = nlohmann::json::parse(r[0][0].as<std::string>());
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while executing query");
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
    Logger::instance().info("Profile endpoint called: " + std::string(req.method_string()));
    if (middleware::rate_limited(ip_address, "/profile", 20))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
      Logger::instance().debug("GET profile requested");
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

      nlohmann::json profile_info = select_profile_data(user_id);
      if (profile_info.empty())
      {
        Logger::instance().info("No profile found for user_id=" + std::to_string(user_id));
        return request::make_bad_request_response("No profile found", req);
      }
      Logger::instance().info("Profile data returned for user_id=" + std::to_string(user_id));

      return request::make_json_request_response(profile_info, req);
    }
    else
    {
      Logger::instance().info("Invalid method for profile endpoint");
      return request::make_bad_request_response("Invalid request method", req);
    }
  }
};

extern "C" RequestHandler *create_profile_handler()
{
  return new ProfileHandler(get_connection_pool());
}