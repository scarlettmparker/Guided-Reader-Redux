#include "api.hpp"
#include "../utils.hpp"

using namespace postgres;
using namespace utils;

class LogoutHandler : public RequestHandler
{
public:
  std::string get_endpoint() const override
  {
    return "/logout";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    Logger::instance().info("Logout endpoint called: " + std::string(req.method_string()));
    if (middleware::rate_limited(ip_address, "/logout", 1))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::post)
    {
      Logger::instance().debug("POST logout requested");
      /**
       * Logout user.
       */
      std::string_view session_id = request::get_session_id_from_cookie(req);
      if (session_id.empty())
      {
        return request::make_unauthorized_response("Invalid or expired session", req);
      }
      if (!request::invalidate_session(std::string(session_id)))
      {
        Logger::instance().error("Failed to invalidate session for logout");
        return request::make_bad_request_response("Failed to invalidate session", req);
      }
      Logger::instance().info("User logged out successfully");

      return request::make_ok_request_response("Successfully logged out", req);
    }
    else
    {
      Logger::instance().info("Invalid method for logout endpoint");
      return request::make_bad_request_response("Invalid request method", req);
    }
  }
};

extern "C" RequestHandler *create_logout_handler()
{
  return new LogoutHandler();
}