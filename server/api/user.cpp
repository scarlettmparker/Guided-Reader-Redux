#include "api.hpp"
#include "bcrypt.h"
#include "../auth/session.hpp"

#include <openssl/rand.h>
#include <ctime>

using namespace postgres;

class UserHandler : public RequestHandler
{
private:
  ConnectionPool &pool;

  /**
   * Helper function to validate an email input. Does not use regex as usually
   * thhat's far too restrictive of an approach (and slow for the back-end).
   *
   * @param email Email to validate.
   * @return true if the email is valid, false otherwise.
   */
  bool validate_email(std::string email)
  {
    if (email.empty())
      return false;
    if (email.length() <= 2)
      return false;

    size_t at_pos = email.find('@');
    if (at_pos == std::string::npos)
      return false;
    std::string domain = email.substr(at_pos + 1);
    size_t dot_pos = domain.find('.');
    size_t dot_count = std::count(domain.begin(), domain.end(), '.');

    if (dot_pos == std::string::npos || dot_pos < 2 || dot_count > 2)
    {
      return false;
    }

    std::vector<std::string> dot_splits;
    std::istringstream iss(domain);
    std::string token;

    while (std::getline(iss, token, '.'))
    {
      dot_splits.push_back(token);
    }

    for (const auto &part : dot_splits)
    {
      if (part.empty())
      {
        return false;
      }
    }

    return true;
  }

  /**
   * Select the ID of a user by username.
   * @param username Username of the user to select.
   * @param verbose Whether to print messages to stdout.
   * @return ID of the user if found, -1 otherwise.
   */
  int select_user_id(std::string username, bool verbose)
  {
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_user_id", username);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        verbose &&std::cout << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty())
      {
        verbose &&std::cout << "User with username " << username << " not found" << std::endl;
        return -1;
      }
      return r[0][0].as<int>();
    }
    catch (const std::exception &e)
    {
      verbose &&std::cout << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while executing query" << std::endl;
    }
    return -1;
  }

  /**
   * Select the email of a user by email. Used to validate if an email is taken or not.
   *
   * @param email Email of the user to select.
   * @param verbose Whether to print messages to stdout.
   * @return Email of the user if found, empty string otherwise.
   */
  std::string select_email(std::string email, bool verbose)
  {
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_email", email);

      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        verbose &&std::cout << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty())
      {
        verbose &&std::cout << "Email " << email << " not found" << std::endl;
        return "";
      }

      return r[0][0].c_str();
    }
    catch (const std::exception &e)
    {
      verbose &&std::cout << "Error executing query: " << e.what() << std::endl;
      return "";
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while executing query" << std::endl;
      return "";
    }
    return "";
  }

  /**
   * Select user data by ID. This returns the username, Discord ID, avatar, and nickname of the user.
   *
   * @param id ID of the user to select.
   * @param verbose Whether to print messages to stdout.
   * @return JSON of user data.
   */
  nlohmann::json select_user_data_by_id(int id, bool verbose)
  {
    nlohmann::json user_data = nlohmann::json::array();

    try
    {
      pqxx::work &txn = request::begin_transaction(pool);

      pqxx::result r = txn.exec_prepared(
          "select_user_data_by_id", id);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        verbose &&std::cout << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty())
      {
        verbose &&std::cout << "User with ID " << id << " not found" << std::endl;
        return user_data;
      }

      user_data = nlohmann::json::parse(r[0][0].as<std::string>());
    }
    catch (const std::exception &e)
    {
      verbose &&std::cout << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while executing query" << std::endl;
    }
    return user_data;
  }

  /**
   * Select a user by ID.
   * @param id ID of the user to select.
   * @param verbose Whether to print messages to stdout.
   * @return Username of the user if found, NULL otherwise.
   */
  std::string select_username_by_id(int id, bool verbose)
  {
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_username_by_id", id);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        verbose &&std::cout << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty())
      {
        verbose &&std::cout << "User with ID " << id << " not found" << std::endl;
        return "";
      }
      return r[0][0].c_str();
    }
    catch (const std::exception &e)
    {
      verbose &&std::cout << "Error executing query: " << e.what() << std::endl;
      return "";
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while executing query" << std::endl;
      return "";
    }
    return "";
  }

  /**
   * Select the password of a user by username.
   * @param username Username of the user to select.
   * @param verbose Whether to print messages to stdout.
   * @return (Hashed) password of the user if found, NULL otherwise.
   */
  std::string select_password(std::string username, bool verbose)
  {
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_user_password", username);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        verbose &&std::cout << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty())
      {
        verbose &&std::cout << "User with username " << username << " not found" << std::endl;
        return "";
      }
      return r[0][0].c_str();
    }
    catch (const std::exception &e)
    {
      verbose &&std::cout << "Error executing query: " << e.what() << std::endl;
      return "";
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while executing query" << std::endl;
      return "";
    }
    return "";
  }

  /**
   * Register a new user with a username and password (non-Discord connected).
   * As user is not connected with Discord, levels is set to {-1} and the discordId to -1.
   *
   * @param username Username of the user to register.
   * @param hashed_password Hashed password of the user to register.
   * @param verbose Whether to print messages to stdout.
   * @return true if the user was registered, false otherwise.
   */
  bool register_user(std::string username, std::string email, std::string hashed_password, bool verbose)
  {
    try
    {
      int current_time = static_cast<int>(std::time(0));
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "insert_user",
          username, email, hashed_password, current_time);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        verbose &&std::cout << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      return true;
    }
    catch (const std::exception &e)
    {
      verbose &&std::cout << "Error executing query: " << e.what() << std::endl;
      return false;
    }
    catch (...)
    {
      verbose &&std::cout << "Unknown error while executing query" << std::endl;
      return false;
    }
  }

  /**
   * Authenticate a user with a username and password.
   * This function uses BCrypt to validate the password against the hashed password stored in the database.
   *
   * @param username Username of the user to authenticate.
   * @param password Password of the user to authenticate.
   * @param verbose Whether to print messages to stdout.
   * @return true if the user is authenticated, false otherwise.
   */
  bool login(std::string username, std::string password)
  {
    std::string stored_password = select_password(username, false);
    if (stored_password.empty())
    {
      return false;
    }
    return bcrypt::validatePassword(password, stored_password);
  }

public:
  UserHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
  }

  std::string get_endpoint() const override
  {
    return "/user";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    if (middleware::rate_limited(ip_address, "/user", 20))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
      /**
       * GET user information.
       */

      std::string_view session_id = request::get_session_id_from_cookie(req);
      if (session_id.empty())
      {
        return request::make_unauthorized_response("Session ID not found", req);
      }
      if (!request::validate_session(std::string(session_id), false))
      {
        return request::make_unauthorized_response("Invalid session ID", req);
      }

      int user_id = request::get_user_id_from_session(std::string(session_id), false);
      if (user_id == -1)
      {
        return request::make_bad_request_response("User not found", req);
      }

      nlohmann::json user_data = select_user_data_by_id(user_id, false);
      if (user_data.empty())
      {
        return request::make_bad_request_response("User not found", req);
      }

      return request::make_json_request_response(user_data, req);
    }
    else if (req.method() == http::verb::post)
    {
      /**
       * Login user.
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

      if (!json_request.contains("username") || !json_request.contains("password"))
      {
        return request::make_bad_request_response("Missing username or password", req);
      }

      if (!json_request["username"].is_string() || !json_request["password"].is_string())
      {
        return request::make_bad_request_response("Invalid username or password", req);
      }

      std::string username = json_request["username"].get<std::string>();
      std::string password = json_request["password"].get<std::string>();

      if (!login(username, password) || password.empty())
      {
        return request::make_unauthorized_response("Invalid username or password", req);
      }

      std::string session_id = session::generate_session_id(false);
      std::string signed_session_id = session_id + "." + session::generate_hmac(session_id, READER_SECRET_KEY);
      int user_id = select_user_id(username, false);

      if (user_id == -1)
      {
        return request::make_bad_request_response("User not found", req);
      }

      int expires_in = std::stoi(READER_SESSION_EXPIRE_LENGTH);
      if (!session::set_session_id(signed_session_id, user_id, expires_in, ip_address, false))
      {
        return request::make_bad_request_response("Failed to set session ID", req);
      }

      return session::set_session_cookie(signed_session_id);
    }
    else if (req.method() == http::verb::put)
    {
      /**
       * PUT new user.
       */
      if (middleware::rate_limited(ip_address, "/register", 0.05))
      {
        return request::make_too_many_requests_response("Too many requests", req);
      }
      nlohmann::json json_request;
      try
      {
        json_request = nlohmann::json::parse(req.body());
      }
      catch (const nlohmann::json::parse_error &e)
      {
        return request::make_bad_request_response("Invalid JSON", req);
      }

      if (!json_request.contains("username") || !json_request.contains("password") || !json_request.contains("email"))
      {
        return request::make_bad_request_response("Please fill in all fields", req);
      }

      if (!json_request["username"].is_string() || !json_request["password"].is_string() || !json_request["email"].is_string())
      {
        return request::make_bad_request_response("Invalid input", req);
      }

      std::string username = json_request["username"].get<std::string>();
      std::string password = json_request["password"].get<std::string>();
      std::string email = json_request["email"].get<std::string>();

      // Validate all the inputs
      if (!validate_email(email))
      {
        return request::make_bad_request_response("Invalid email", req);
      }
      if (password.length() < 8)
      {
        return request::make_bad_request_response("Password too short", req);
      }

      if (select_user_id(username, false) != -1)
      {
        return request::make_bad_request_response("Username taken", req);
      }
      if (!select_email(email, false).empty())
      {
        return request::make_bad_request_response("Email taken", req);
      }

      std::string hashed_password = bcrypt::generateHash(password);
      if (hashed_password.empty())
      {
        return request::make_bad_request_response("Failed to hash password", req);
      }

      if (!register_user(username, email, hashed_password, false))
      {
        return request::make_bad_request_response("Failed to register user", req);
      }

      return request::make_ok_request_response("User registered", req);
    }
    else if (req.method() == http::verb::patch)
    {
      /**
       * Update user information.
       */
    }
    else if (req.method() == http::verb::delete_)
    {
      /**
       * DELETE user.
       */
    }
    else
    {
      return request::make_bad_request_response("Invalid request method", req);
    }
  }
};

extern "C" RequestHandler *create_user_handler()
{
  return new UserHandler(get_connection_pool());
}