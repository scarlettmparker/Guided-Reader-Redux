#include "middleware.hpp"
#include "../utils.hpp"

using namespace postgres;
namespace middleware
{
  std::mutex rate_limit_mutex;
  std::unordered_map<CacheKey, RateLimitData> rate_limit_cache;

  /**
   * Check if a user is being rate limited.
   * This works by checking if enough milliseconds have passed since the last request.
   *
   * @param ip_address IP address of the user to check.
   * @param endpoint The API endpoint being accessed
   * @param window_ms Time window in milliseconds between allowed requests
   * @return true if the user is rate limited, false otherwise.
   */
  /**
   * Check if a user is being rate limited.
   * This works by checking if the user is making too many requests per second.
   *
   * @param ip_address IP address of the user to check.
   * @param endpoint The API endpoint being accessed.
   * @param max_requests_per_second Maximum allowed requests per second.
   * @return true if the user is rate limited, false otherwise.
   */
  bool rate_limited(const std::string &ip_address, const std::string &endpoint, float max_requests_per_second)
  {
    std::lock_guard<std::mutex> guard(rate_limit_mutex);
    auto now = std::chrono::system_clock::now();
    CacheKey key{ip_address, endpoint};
    auto &data = rate_limit_cache[key];

    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch())
                      .count();

    while (!data.request_timestamps.empty() &&
           now_ms - data.request_timestamps.front() >= 1000)
    {
      data.request_timestamps.pop_front();
    }

    if (data.request_timestamps.size() >= max_requests_per_second)
    {
      return true;
    }

    data.request_timestamps.push_back(now_ms);
    return false;
  }

  /**
   * Check if a user has accepted the privacy policy. This is used to block
   * usage of certain API endpoints until the user has accepted the policy.
   *
   * @param user_id ID of the user to check.
   * @return true if the user has accepted the policy, false otherwise.
   */
  bool user_accepted_policy(const int user_id)
  {
    try
    {
      auto &pool = get_connection_pool();
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

      if (r.empty())
      {
        utils::Logger::instance().debug("Policy not accepted for user_id=" + std::to_string(user_id));
        return false;
      }
      if (r[0][0].as<bool>())
      {
        return true;
      }
      return false;
    }
    catch (const std::exception &e)
    {
      utils::Logger::instance().error(std::string("Error checking policy acceptance: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while checking policy acceptance");
    }
    return false;
  }
}