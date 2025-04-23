#ifndef EMAIL_HPP
#define EMAIL_HPP

#include <string>
#include <curl/curl.h>
#include <stdexcept>
#include <sstream>
#include <mutex>
#include <memory>
#include <iostream>
#include <cstring>
#include <ctime>
#include <openssl/rand.h>
#include <iomanip>

#include "../db/redis.hpp"

namespace email
{
  struct email_config
  {
    std::string host;
    int port;
    std::string username;
    std::string password;
    bool use_tls;
  };

  bool validate_recovery_code(int user_id, const std::string &recovery_code, bool verbose);
  bool insert_recovery_code(int user_id, const std::string &recovery_code, bool verbose);
  std::string generate_recovery_code();
  std::string get_rfc822_date();

  class SMTPClient
  {
  public:
    SMTPClient(const std::string &host, int port, bool use_tls = true);
    ~SMTPClient();

    void connect();
    void login(const std::string &username, const std::string &password);
    void send_mail(const std::string &from,
                   const std::string &to,
                   const std::string &subject,
                   const std::string &body);
    void disconnect();

  private:
    CURL *curl_;
    std::string host_;
    int port_;
    bool use_tls_;
    bool is_connected_;
  };

  class EmailService
  {
  public:
    static EmailService &get_instance();
    void configure(const email_config &config);
    void send_email(const std::string &from,
                    const std::string &to,
                    const std::string &subject,
                    const std::string &body);
    ~EmailService();

  private:
    EmailService() = default;
    EmailService(const EmailService &) = delete;
    EmailService &operator=(const EmailService &) = delete;

    std::unique_ptr<SMTPClient> client_;
    std::mutex mutex_;
    bool is_configured_ = false;
  };
}

#endif