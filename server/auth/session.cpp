#include "session.hpp"

namespace session
{
  /**
   * Generate a session ID for a user. This function uses OpenSSL to generate a random 128-bit session ID.
   * @param verbose Whether to print messages to stdout.
   * @return Session ID as a string.
   */
  std::string generate_session_id(bool verbose)
  {
    unsigned char buffer[16];
    if (RAND_bytes(buffer, sizeof(buffer)) != 1)
    {
      verbose &&std::cerr << "Failed to generate session ID" << std::endl;
    }

    std::stringstream session_id;
    for (int i = 0; i < 16; ++i)
    {
      session_id << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
    }

    return session_id.str();
  }

  /**
   * Set a session cookie for a user. This ets the session ID in a cookie with the following attributes:
   * - HttpOnly: The cookie is not accessible via JavaScript.
   * - Secure: The cookie is only sent over HTTPS.
   * - SameSite=None: The cookie is sent with cross-site requests.
   * - Max-Age=86400: The cookie expires after 24 hours.
   *
   * @param signed_session_id Session ID to set in the cookie.
   * @return HTTP response with the session cookie set.
   */
  http::response<http::string_body> set_session_cookie(const std::string &signed_session_id)
  {
    http::response<http::string_body> res{http::status::ok, 11};

    res.set(http::field::content_type, "application/json");
    res.set(http::field::set_cookie, "sessionId=" + signed_session_id + "; HttpOnly; Secure; SameSite=Strict; Max-Age=86400");
    res.body() = R"({"message": "Login successful", "status": "ok"})";
    res.prepare_payload();

    return res;
  }

  /**
   * Set a session ID for a user.
   * @param signed_session_id Session ID to set.
   * @param user_id ID of the user to set the session ID for.
   * @param username Username of the user to set the session ID for.
   * @param duration Duration of the session in seconds.
   * @param ip_address IP address of the user.
   * @param verbose Whether to print messages to stdout.
   * @return true if the session ID was set, false otherwise.
   */
  bool set_session_id(std::string signed_session_id, int user_id, int duration, std::string ip_address, bool verbose)
  {
    try
    {
      sw::redis::Redis &redis = Redis::get_instance();
      auto now = std::chrono::system_clock::now();

      std::time_t created_at = std::chrono::system_clock::to_time_t(now);
      std::time_t expires_at = created_at + duration;

      std::unordered_map<std::string, std::string> session_data =
          {
              {"user_id", std::to_string(user_id)},
              {"created_at", std::to_string(created_at)},
              {"expires_at", std::to_string(expires_at)},
              {"ip_address", ip_address}};

      std::string key = "session:" + signed_session_id;

      try
      {
        redis.hmset(key, session_data.begin(), session_data.end());
      }
      catch (const sw::redis::Error &e)
      {
        verbose &&std::cerr << "Failed to set session hash in Redis: " << e.what() << std::endl;
        return false;
      }

      if (!redis.expire(key, duration))
      {
        verbose &&std::cerr << "Failed to set session expiration in Redis" << std::endl;
        return false;
      }

      if (!redis.sadd("user:" + std::to_string(user_id) + ":sessions", signed_session_id))
      {
        verbose &&std::cerr << "Failed to add session ID to user sessions set in Redis" << std::endl;
        return false;
      }

      return true;
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error setting session ID: " << e.what() << std::endl;
      return false;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while setting session ID" << std::endl;
      return false;
    }
  }

  /**
   * Helper function to convert a byte string to a hex string.
   * This is used to prevent issues with encoding differences as the HMAC output is raw binary data,
   * possibly containing null bytes/special characters, and should therefore be encoded to hex.
   *
   * @param bytes Byte string to convert to hex.
   * @return Hex string.
   */
  std::string bytes_to_hex(const std::string &bytes)
  {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : bytes)
    {
      ss << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }
    return ss.str();
  }

  /**
   * Generate an HMAC for a given data string using a key. This is used to sign session IDs.
   * This works by using the OpenSSL EVP_MAC functions to generate an HMAC.
   *
   * @param data Data to generate the HMAC for.
   * @param key Key to use for the HMAC.
   * @return HMAC for the data.
   */
  std::string generate_hmac(const std::string &data, const std::string &key)
  {
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (!mac)
    {
      throw std::runtime_error("Failed to create MAC");
    }

    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(mac);
    if (!ctx)
    {
      EVP_MAC_free(mac);
      throw std::runtime_error("Failed to create MAC context");
    }

    OSSL_PARAM params[2];

    // Set the digest to SHA256
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char *>("SHA256"), 6);
    params[1] = OSSL_PARAM_construct_end();

    if (!EVP_MAC_init(ctx, reinterpret_cast<const unsigned char *>(key.c_str()), key.length(), params))
    {
      EVP_MAC_CTX_free(ctx);
      EVP_MAC_free(mac);
      throw std::runtime_error("Failed to initialize MAC");
    }

    if (!EVP_MAC_update(ctx, reinterpret_cast<const unsigned char *>(data.c_str()), data.length()))
    {
      EVP_MAC_CTX_free(ctx);
      EVP_MAC_free(mac);
      throw std::runtime_error("Failed to update MAC");
    }

    size_t out_len;
    if (!EVP_MAC_final(ctx, nullptr, &out_len, 0))
    {
      EVP_MAC_CTX_free(ctx);
      EVP_MAC_free(mac);
      throw std::runtime_error("Failed to get MAC length");
    }

    std::vector<unsigned char> result(out_len);
    if (!EVP_MAC_final(ctx, result.data(), &out_len, result.size()))
    {
      EVP_MAC_CTX_free(ctx);
      EVP_MAC_free(mac);
      throw std::runtime_error("Failed to get MAC");
    }

    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);

    return bytes_to_hex(std::string(reinterpret_cast<char *>(result.data()), out_len));
  }
}