#include "redis.hpp"

std::unique_ptr<sw::redis::Redis> Redis::instance_ = nullptr;

/**
 * Initialize the Redis connection. This will create a new connection pool of
 * size 10 using the .env specified host and port.
 */
void Redis::init_connection()
{
  if (!instance_)
  {
    sw::redis::ConnectionOptions connection_opts;
    connection_opts.host = READER_REDIS_HOST;
    connection_opts.port = std::stoi(READER_REDIS_PORT);
    connection_opts.keep_alive = true;
    connection_opts.connect_timeout = std::chrono::seconds(5);

    sw::redis::ConnectionPoolOptions pool_opts;
    pool_opts.size = 10;

    instance_ = std::make_unique<sw::redis::Redis>(connection_opts, pool_opts);
    std::cout << "Connected to Redis server" << std::endl;
  }
}

/**
 * Get the Redis instance.
 * @return Redis instance.
 */
sw::redis::Redis &Redis::get_instance()
{
  if (!instance_)
  {
    throw std::runtime_error("Redis not initialized");
  }
  return *instance_;
}