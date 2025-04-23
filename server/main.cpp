#include "server.hpp"
#include "auth/email.hpp"
#include "db/redis.hpp"
#include "db/postgres.hpp"
#include "config.h"

int main()
{
  try
  {
    auto const address = net::ip::make_address(READER_SERVER_HOST);
    unsigned short port = static_cast<unsigned short>(READER_SERVER_PORT);

    std::cout << "Starting server on " << address << ":" << port << std::endl;

    net::io_context ioc;
    std::vector<std::thread> threads;

    auto listener = std::make_shared<server::Listener>(ioc, tcp::endpoint{address, port});

    /**
     * Initialize PostgreSQL connection.
     */
    postgres::init_connection();

    /**
     * Initialize Redis connection.
     */
    Redis::init_connection();

    /**
     * Initialize email service.
     */
    email::email_config config{
        READER_EMAIL_HOST,
        READER_EMAIL_PORT,
        READER_EMAIL_ADDRESS,
    };

    auto &service = email::EmailService::get_instance();
    service.configure(config);

    for (std::size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
      threads.emplace_back([&ioc]
                           { ioc.run(); });
    }

    for (auto &t : threads)
    {
      t.join();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}