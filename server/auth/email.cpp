#include "email.hpp"
#include "../utils.hpp"

namespace email
{
  /**
   * Validate a recovery code for a user. This is used to recover a lost password.
   *
   * @param user_id ID of the user to validate the recovery code for.
   * @param recovery_code Recovery code to validate.
   * @return true if the recovery code is valid, false otherwise.
   */
  bool validate_recovery_code(int user_id, const std::string &recovery_code)
  {
    auto &redis = Redis::get_instance();
    std::string key = "recovery:" + std::to_string(user_id);
    try
    {
      auto val = redis.get(key);
      if (!val)
      {
        return false;
      }
      auto ttl = redis.ttl(key);
      if (ttl < 0)
      {
        redis.del(key);
        return false;
      }

      return *val == recovery_code;
    }
    catch (const sw::redis::Error &e)
    {
    }
    catch (...)
    {
    }
    return false;
  }

  /**
   * Insert a recovery code for a user into Redis. This is used to recover a lost password.
   * It has a TTL (expiration time) of 5 minutes.
   *
   * @param user_id ID of the user to insert the recovery code for.
   * @param recovery_code Recovery code to insert.
   * @return true if the recovery code was inserted, false otherwise.
   */
  bool insert_recovery_code(int user_id, const std::string &recovery_code)
  {
    auto &redis = Redis::get_instance();
    std::string key = "recovery:" + std::to_string(user_id);
    try
    {
      if (!redis.set(key, recovery_code))
      {
        return false;
      }
      if (!redis.expire(key, 300))
      {
        redis.del(key);
        return false;
      }
      return true;
    }
    catch (const sw::redis::Error &e)
    {
    }
    catch (...)
    {
    }
    return false;
  }

  /**
   * Generate a random recovery code for a user. This is used to recover a
   * lost password and will be sent in an email.
   *
   * @return Recovery code as a string.
   */
  std::string generate_recovery_code()
  {
    unsigned char buffer[8];
    if (RAND_bytes(buffer, sizeof(buffer)) != 1)
    {
      throw std::runtime_error("Failed to generate recovery code");
    }

    std::stringstream code;
    for (int i = 0; i < 8; ++i)
    {
      code << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
    }

    return code.str();
  }

  /**
   * Helper function to get the current date in RFC 822 format.
   * Used to set the Date header in the email.
   *
   * @return Current date in RFC 822 format.
   */
  std::string get_rfc822_date()
  {
    char buffer[100];
    std::time_t now = std::time(nullptr);
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %z", std::localtime(&now));
    return buffer;
  }

  /**
   * @brief Retrieves a new Gmail OAuth2 access token using the configured refresh token.
   *
   * This makes a POST request to the Gmail OAuth2 token endpoint
   * with the credentials and refresh token from env to obtain a fresh access token.
   *
   * @return Access token string if successful.
   */
  std::string get_access_token()
  {

    httpclient::HTTPClient client{READER_EMAIL_OAUTH, "443", true};
    client.set_content_type("application/x-www-form-urlencoded");

    std::stringstream body;
    body << "client_id=" << READER_EMAIL_CLIENT_ID
         << "&client_secret=" << READER_EMAIL_CLIENT_SECRET
         << "&refresh_token=" << READER_EMAIL_REFRESH_TOKEN
         << "&grant_type=refresh_token";

    std::string response = client.post("/token", body.str());
    nlohmann::json json = nlohmann::json::parse(response);

    if (json.contains("access_token"))
    {
      return json["access_token"];
    }
    else
    {
      throw std::runtime_error("Failed to get access token!");
    }
  }

  /**
   * @brief Callback function used by libcurl to read email payload data from memory.
   *
   * This is called repeatedly by libcurl when sending the email body via SMTP.
   * It reads from the internal string buffer (payload_) of the SMTPClient instance,
   * copying data into `ptr` for libcurl to send.
   *
   * @param ptr Pointer to the buffer where data should be copied.
   * @param size Size of a single element.
   * @param nmemb Number of elements to write.
   * @param userp Pointer to the SMTPClient instance (cast from void*).
   * @return size_t The number of bytes copied into ptr (may be less than size * nmemb on the last call).
   */
  static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
  {
    SMTPClient *client = static_cast<SMTPClient *>(userp);
    size_t buffer_size = size * nmemb;
    size_t remaining = client->payload_.size() - client->payload_pos_;
    size_t to_copy = std::min(buffer_size, remaining);

    if (to_copy > 0)
    {
      std::memcpy(ptr, client->payload_.c_str() + client->payload_pos_, to_copy);
      client->payload_pos_ += to_copy;
    }

    return to_copy;
  }

  SMTPClient::SMTPClient(const std::string &host, int port, bool use_tls)
      : host_(host), port_(port), use_tls_(use_tls), is_connected_(false)
  {
    curl_ = curl_easy_init();
    if (!curl_)
    {
      throw std::runtime_error("Failed to initialize CURL");
    }
  }

  SMTPClient::~SMTPClient()
  {
    if (is_connected_)
    {
      disconnect();
    }
    curl_easy_cleanup(curl_);
  }

  /**
   * Connect to the SMTP server.
   * This method is called before sending an email.
   */
  void SMTPClient::connect()
  {
    if (is_connected_)
    {
      return;
    }

    std::string url = (use_tls_ ? "smtps://" : "smtp://") + host_ + ":" + std::to_string(port_);
    curl_easy_reset(curl_);

    // Set options
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl_, CURLOPT_PROTOCOLS, CURLPROTO_SMTPS);
    curl_easy_setopt(curl_, CURLOPT_DEFAULT_PROTOCOL, "smtps");
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 60L);

    is_connected_ = true;
  }

  /**
   * Set libcurl opts for OAuth2 username and access token using XOAUTH2
   *
   * @param email Email address (hi@scarlettparker.co.uk)
   * @param access_token Access token from get_access_token()
   */
  void SMTPClient::set_oauth2_opts(const std::string &email, const std::string &access_token)
  {
    if (!is_connected_)
    {
      throw std::runtime_error("Not connected to SMTP server");
    }

    curl_easy_setopt(curl_, CURLOPT_USERNAME, email.c_str());
    curl_easy_setopt(curl_, CURLOPT_LOGIN_OPTIONS, "AUTH=XOAUTH2");
    curl_easy_setopt(curl_, CURLOPT_XOAUTH2_BEARER, access_token.c_str());
  }

  /**
   * Send an email. This works by building the email content and then using
   * CURL to send the email to the SMTP server.
   *
   * @param from Email address of the sender.
   * @param to Email address of the recipient.
   * @param subject Subject of the email.
   * @param body Body of the email.
   */
  void SMTPClient::send_mail(const std::string &from,
                             const std::string &to,
                             const std::string &subject,
                             const std::string &body)
  {
    if (!is_connected_)
    {
      throw std::runtime_error("Not connected to SMTP server");
    }

    struct curl_slist *recipients = NULL;
    recipients = curl_slist_append(recipients, to.c_str());

    // Build the email
    std::stringstream email_content;
    email_content << "Date: " << get_rfc822_date() << "\r\n";
    email_content << "To: " << to << "\r\n";
    email_content << "From: " << from << "\r\n";
    email_content << "Subject: " << subject << "\r\n";
    email_content << "Content-Type: text/plain; charset=UTF-8\r\n";
    email_content << "\r\n";
    email_content << body;

    std::string payload = email_content.str();
    payload_ = payload;
    payload_pos_ = 0;

    curl_easy_setopt(curl_, CURLOPT_MAIL_FROM, from.c_str());
    curl_easy_setopt(curl_, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl_, CURLOPT_READDATA, this);
    curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(recipients);

    if (res != CURLE_OK)
    {
      std::string error_msg = "Failed to send email: ";
      error_msg += curl_easy_strerror(res);
      throw std::runtime_error(error_msg);
    }
  }

  /**
   * Disconnect from the SMTP server.
   */
  void SMTPClient::disconnect()
  {
    if (is_connected_)
    {
      curl_easy_reset(curl_);
      is_connected_ = false;
    }
  }

  /**
   * Get the singleton instance of the EmailService.
   *
   * @return Reference to the EmailService instance.
   */
  EmailService &EmailService::get_instance()
  {
    static EmailService instance;
    return instance;
  }

  /**
   * Configure the EmailService with the given configuration.
   *
   * @param config Configuration for the email service.
   */
  void EmailService::configure(const email_config &config)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    try
    {
      client_ = std::make_unique<SMTPClient>(config.host, config.port, true);
      client_->connect();

      std::string access_token = get_access_token();
      client_->set_oauth2_opts(config.username, access_token);

      is_configured_ = true;
      std::cout << "Email service configured with OAuth2" << std::endl;
    }
    catch (const std::exception &e)
    {
      // std::cerr << "Error in configuring email service: " << e.what() << std::endl;
    }
  }

  /**
   * Send an email using the configured SMTP server.
   *
   * @param from Email address of the sender.
   * @param to Email address of the recipient.
   * @param subject Subject of the email.
   * @param body Body of the email.
   */
  void EmailService::send_email(const std::string &from,
                                const std::string &to,
                                const std::string &subject,
                                const std::string &body)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_configured_)
    {
      throw std::runtime_error("Email service not configured");
    }
    client_->send_mail(from, to, subject, body);
  }

  EmailService::~EmailService()
  {
    if (client_)
    {
      client_->disconnect();
    }
  }
}