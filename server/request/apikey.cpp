#include "apikey.hpp"

namespace apikey
{
  /**
   * Check if an API key exists in Redis. Used to verify API keys sent from the front-end.
   *
   * @param api_key API key to check.
   * @return true if the API key exists, false otherwise.
   */
  bool api_key_exists(const std::string &api_key)
  {
    sw::redis::Redis &redis = Redis::get_instance();
    try
    {
      return redis.exists(api_key);
    }
    catch (const sw::redis::Error &e)
    {
      std::cerr << "Error checking if API key exists in Redis: " << e.what() << std::endl;
      return false;
    }
  }

  /**
   * Get the details of an API key from Redis. This function retrieves the request limit,
   * permissions, and request count for the given API key.
   *
   * @param api_key API key to get details for.
   * @return Details of the API key.
   */
  APIKey get_api_key_details(const std::string &api_key)
  {
    sw::redis::Redis &redis = Redis::get_instance();
    APIKey details;
    details.key = api_key;

    try
    {
      std::optional<std::string> request_limit_str = redis.hget(api_key, "request_limit");
      std::optional<std::string> permissions_str = redis.hget(api_key, "permissions");
      details.request_limit = request_limit_str ? std::stoi(*request_limit_str) : 0;
      details.requests_last_24h = get_request_count(api_key);

      // Split permissions string into vector by comma separator
      if (permissions_str)
      {
        std::vector<std::string> perms;
        boost::split(perms, *permissions_str, boost::is_any_of(","));
        details.permissions = perms;
      }

      return details;
    }
    catch (const sw::redis::Error &e)
    {
      std::cerr << "Error getting API key details from Redis: " << e.what() << std::endl;
      return APIKey();
    }
  }

  /**
   * Get the request count for an API key. This function retrieves the number of requests
   * made with the given API key in the last 24 hours. This is used to enforce daily rate limiting.
   *
   * @param api_key API key to get the request count for.
   * @return Number of requests made with the API key in the last 24 hours.
   */
  int get_request_count(const std::string &api_key)
  {
    sw::redis::Redis &redis = Redis::get_instance();
    try
    {
      auto now = std::chrono::system_clock::now();
      auto one_day_ago = now - std::chrono::hours(24);
      auto min_score = std::chrono::duration_cast<std::chrono::seconds>(one_day_ago.time_since_epoch()).count();

      double min_score_d = static_cast<double>(min_score);
      double zero_score = 0.0;

      // Create a bounded interval from 0 to min_score_d
      auto interval = sw::redis::BoundedInterval<double>(
          zero_score,                  // min
          min_score_d,                 // max
          sw::redis::BoundType::CLOSED // include boundaries
      );

      redis.zremrangebyscore(api_key + ":requests", interval);
      return redis.zcard(api_key + ":requests");
    }
    catch (const sw::redis::Error &e)
    {
      std::cerr << "Error getting request count from Redis: " << e.what() << std::endl;
      return -1;
    }
  }

  /**
   * Increment the request count for an API key. This function adds the current timestamp
   * to the sorted set of requests for the given API key. This is used to track the number
   * of requests made with the API key in the last 24 hours.
   *
   * @param api_key API key to increment the request count for.
   * @return true if the request count was incremented, false otherwise.
   */
  bool increment_request_count(const std::string &api_key)
  {
    sw::redis::Redis &redis = Redis::get_instance();
    try
    {
      auto now = std::chrono::system_clock::now();
      auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
      double score = static_cast<double>(timestamp);

      std::pair<std::string, double> member_score = {std::to_string(timestamp), score};
      redis.zadd(api_key + ":requests", {member_score});
      return true;
    }
    catch (const sw::redis::Error &e)
    {
      std::cerr << "Error incrementing request count in Redis: " << e.what() << std::endl;
      return false;
    }
  }

  /**
   * Generate a new API key. This function generates a random string of the given length
   * and checks if it already exists in Redis. If it does, it will generate a new key.
   * This will generate a RFC4122 version 4 compliant UUID key.
   *
   * @return New API key.
   */
  std::string generate_api_key()
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    uint8_t data[16];
    for (int i = 0; i < 16; i++)
    {
      data[i] = dis(gen);
    }

    data[6] = (data[6] & 0x0F) | 0x40;
    data[8] = (data[8] & 0x3F) | 0x80;

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < 16; i++)
    {
      ss << std::setw(2) << static_cast<int>(data[i]);
      if (i == 3 || i == 5 || i == 7 || i == 9)
      {
        ss << '-';
      }
    }

    std::string api_key = ss.str();

    // Assume api key hasn't been generated yet
    // Btw if this ever happens i am literally god
    while (api_key_exists(api_key))
    {
      return generate_api_key();
    }

    return api_key;
  }

  /**
   * Insert a new API key into Redis. This function sets the request limit and permissions
   * for the given API key.
   *
   * @param api_key API key to insert.
   * @param request_limit Request limit for the API key.
   * @param permissions Permissions for the API key.
   * @return true if the API key was inserted, false otherwise.
   */
  bool insert_api_key(const std::string &api_key, int request_limit, const std::vector<std::string> &permissions)
  {
    sw::redis::Redis &redis = Redis::get_instance();
    try
    {
      redis.hset(api_key, "request_limit", std::to_string(request_limit));
      redis.hset(api_key, "permissions", boost::algorithm::join(permissions, ","));
      return true;
    }
    catch (const sw::redis::Error &e)
    {
      std::cerr << "Error inserting API key into Redis: " << e.what() << std::endl;
      return false;
    }
  }

  /**
   * Update an API key in Redis. This function sets the request limit and
   * permissions for the given API key, if for whatever reason they need to be changed.
   *
   * @param api_key API key to update.
   * @param request_limit New request limit for the API key.
   * @param permissions New permissions for the API key.
   * @return true if the API key was updated, false otherwise.
   */
  bool update_api_key(const std::string &api_key, int request_limit, const std::vector<std::string> &permissions)
  {
    sw::redis::Redis &redis = Redis::get_instance();
    try
    {
      redis.hset(api_key, "request_limit", std::to_string(request_limit));
      redis.hset(api_key, "permissions", boost::algorithm::join(permissions, ","));
      return true;
    }
    catch (const sw::redis::Error &e)
    {
      std::cerr << "Error updating API key in Redis: " << e.what() << std::endl;
      return false;
    }
  }

  /**
   * Destroy an API key in Redis. This function removes the API key and all associated data
   * from Redis. This is needed if an API key is no longer needed or has been compromised.
   *
   * @param api_key API key to destroy.
   * @return true if the API key was destroyed, false otherwise.
   */
  bool destroy_api_key(const std::string &api_key)
  {
    sw::redis::Redis &redis = Redis::get_instance();
    try
    {
      redis.del(api_key);
      return true;
    }
    catch (const sw::redis::Error &e)
    {
      std::cerr << "Error destroying API key in Redis: " << e.what() << std::endl;
      return false;
    }
  }

  /**
   * Verify an API key. This function checks if the given API key exists in Redis,
   * has not exceeded its request limit, and is valid for the given request.
   *
   * @param req HTTP request to verify the API key for.
   * @return true if the API key is valid, false otherwise.
   */
  bool verify_api_key(const http::request<http::string_body> &req)
  {
    std::string api_key = req[http::field::authorization].to_string();
    api_key.erase(0, 7); // Bearer

    if (api_key.empty())
    {
      return false;
    }
    if (!api_key_exists(api_key))
    {
      return false;
    }

    APIKey details = get_api_key_details(api_key);
    if (details.key.empty())
    {
      return false;
    }
    if (details.request_limit > 0 && details.requests_last_24h >= details.request_limit)
    {
      return false;
    }

    increment_request_count(api_key);
    return true;
  }
}