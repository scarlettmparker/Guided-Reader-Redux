#ifndef UTILS_HPP
#define UTILS_HPP

#include "config.h"
#include <string>
#include <optional>
#include <mutex>
#include <memory>
#include <iostream>
#include <regex>
#include <fstream>
#include <algorithm>
#include <cctype>

namespace utils
{
  /**
   * @brief Log levels for the Logger.
   */
  enum class LogLevel
  {
    Error = 0,
    Info = 1,
    Debug = 2
  };

  /**
   * @brief Simple thread-safe logger with log level control.
   *
   * Usage: utils::Logger::instance().info("message");
   */
  class Logger
  {
  public:
    static Logger &instance();
    void set_level(LogLevel level);
    LogLevel get_level() const;
    void error(const std::string &msg);
    void info(const std::string &msg);
    void debug(const std::string &msg);
    void initialize_from_env();

  private:
    Logger();
    LogLevel level_;
    std::mutex mutex_;
  };
}

#endif // UTILS_HPP
