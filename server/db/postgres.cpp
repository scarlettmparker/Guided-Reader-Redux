#include "postgres.hpp"

namespace postgres
{
  static ConnectionPool *global_pool = nullptr;
  std::unordered_map<pqxx::connection *, ConnectionMetadata> connection_metadata;

  /**
   * Create a new connection for the connection pool.
   * @return New connection.
   */
  pqxx::connection *ConnectionPool::create_new_connection()
  {
    auto c = new pqxx::connection(
        "user=" + std::string(READER_DB_USERNAME) +
        " password=" + std::string(READER_DB_PASSWORD) +
        " host=" + std::string(READER_DB_HOST) +
        " port=" + std::string(READER_DB_PORT) +
        " dbname=" + std::string(READER_DB_NAME) +
        " target_session_attrs=read-write" +
        " keepalives=1" +
        " keepalives_idle=30");

    if (!c->is_open())
    {
      delete c;
      throw std::runtime_error("Failed to open PostgreSQL connection!");
    }

    return c;
  }

  /**
   * Create a new connection pool with a given size.
   * @param size Size of the connection pool.
   */
  ConnectionPool::ConnectionPool(int size) : max_size(size)
  {
    for (int i = 0; i < size; ++i)
    {
      pool.push(create_new_connection());
    }
  }

  /**
   * Destroy the connection pool and free all connections.
   */
  ConnectionPool::~ConnectionPool()
  {
    std::lock_guard<std::mutex> lock(pool_mutex);
    while (!pool.empty())
    {
      auto c = pool.front();
      pool.pop();
      delete c;
    }
    connection_metadata.clear();
  }

  /**
   * Validate a connection by executing a simple query.
   * @param conn Connection to validate.
   */
  int ConnectionPool::validate_connection(pqxx::connection *c)
  {
    try
    {
      return c->is_open();
    }
    catch (...)
    {
      return false;
    }
  }

  /**
   * Acquire a connection from the pool. This function will block until a connection is available.
   * If a connection is not used for more than 1 minute, it will be released.
   *
   * @return Connection from the pool.
   */
  pqxx::connection *ConnectionPool::acquire()
  {
    pqxx::connection *c = nullptr;
    auto now = std::chrono::steady_clock::now();

    {
      std::unique_lock<std::mutex> lock(pool_mutex);
      bool got_connection = pool_cv.wait_for(
          lock, std::chrono::milliseconds(ACQUIRE_TIMEOUT_MS), [this]
          { return !pool.empty(); });

      if (!got_connection)
      {
        failed_acquires++;
        throw std::runtime_error("Connection pool timeout");
      }

      c = pool.front();
      pool.pop();
      active_connections++;
    }

    for (int retry = 0; retry < MAX_RETRIES; retry++)
    {
      // Health check
      ConnectionMetadata &metadata = connection_metadata[c];
      const auto connection_age = std::chrono::duration_cast<std::chrono::minutes>(
                                      now - metadata.last_used)
                                      .count();
      const auto last_health_check = std::chrono::duration_cast<std::chrono::seconds>(
                                         now - metadata.last_checked)
                                         .count();

      if (connection_age > CONNECTION_LIFETIME_MIN || last_health_check > HEALTH_CHECK_INTERVAL_SEC)
      {
        metadata.is_healthy = validate_connection(c);
        metadata.last_checked = now;

        if (!metadata.is_healthy)
        {
          delete c;
          try
          {
            // Attempt to create a new connection
            c = create_new_connection();
            connection_metadata[c] = {now, now, true};
            return c;
          }
          catch (const std::exception &e)
          {
            if (retry == MAX_RETRIES - 1)
            {
              active_connections--;
              throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (retry + 1)));
            continue;
          }
        }
      }
      metadata.last_used = now;
      return c;
    }

    active_connections--;
    throw std::runtime_error("Failed to acquire connection");
  }

  /**
   * Release a connection back to the pool.
   * @param c Connection to release.
   */
  void ConnectionPool::release(pqxx::connection *c)
  {
    std::lock_guard<std::mutex> lock(pool_mutex);
    if (active_connections > 0)
    {
      active_connections--;
    }
    pool.push(c);
    pool_cv.notify_one();
  }

  /**
   * Initialize the global connection pool.
   */
  void init_connection()
  {
    if (!global_pool)
    {
      global_pool = new ConnectionPool(std::max(10u, 2 * std::thread::hardware_concurrency()));
    }
    std::cout << "Postgres connection pool initialized with " << global_pool->max_size << " connections." << std::endl;
  }

  /**
   * Get the global connection pool.
   * @return Global connection pool.
   */
  ConnectionPool &get_connection_pool()
  {
    if (!global_pool)
    {
      throw std::runtime_error("Connection pool not initialized. Call init_connection first.");
    }
    return *global_pool;
  }
}
