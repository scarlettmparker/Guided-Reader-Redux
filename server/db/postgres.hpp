#ifndef POSTGRES_HPP
#define POSTGRES_HPP

#include <pqxx/pqxx>
#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <algorithm>

#include "config.h"

namespace postgres
{
  struct ConnectionMetadata
  {
    std::chrono::time_point<std::chrono::steady_clock> last_used;
    std::chrono::time_point<std::chrono::steady_clock> last_checked;
    bool is_healthy;
  };

  class ConnectionPool
  {
  private:
    const int MAX_CONNECTIONS = 20;
    const int ACQUIRE_TIMEOUT_MS = 5000;
    const int CONNECTION_LIFETIME_MIN = 30;
    const int HEALTH_CHECK_INTERVAL_SEC = 30;
    const int MAX_RETRIES = 3;

    std::atomic<int> active_connections{0};
    std::atomic<int> failed_acquires{0};

    std::unordered_map<std::string, std::string> prepared_statements;
    std::queue<pqxx::connection *> pool;
    std::mutex pool_mutex;
    std::condition_variable pool_cv;

    pqxx::connection *create_new_connection();
    int validate_connection(pqxx::connection *c);

  public:
    int max_size;
    explicit ConnectionPool(int size);
    ~ConnectionPool();

    ConnectionPool(const ConnectionPool &) = delete;
    ConnectionPool &operator=(const ConnectionPool &) = delete;

    pqxx::connection *acquire();
    void release(pqxx::connection *c);
  };

  extern std::unordered_map<pqxx::connection *, ConnectionMetadata> connection_metadata;
  void init_connection();
  ConnectionPool &get_connection_pool();
}

#endif
