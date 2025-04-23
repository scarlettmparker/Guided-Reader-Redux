#include "server.hpp"

namespace server
{
  /**
   * Load all request handlers from the specified directory.
   * This function loads all shared objects (.so files) from the specified directory and
   * looks for a function named create_<handler_name>_handler in each of them.
   *
   * @param directory Directory to load handlers from.
   * @return Vector of unique pointers to the loaded request handlers.
   */
  std::vector<std::unique_ptr<RequestHandler>> load_handlers(const std::string &directory)
  {
    std::vector<std::unique_ptr<RequestHandler>> handlers;

    for (std::filesystem::directory_entry const &entry : std::filesystem::directory_iterator(directory))
    {
      if (entry.path().extension() == ".so")
      {
        void *lib_handle = dlopen(entry.path().c_str(), RTLD_LAZY);
        if (!lib_handle)
        {
          throw std::runtime_error("Failed to load library: " + entry.path().string());
        }

        std::string filename = entry.path().stem().string();
        if (filename.rfind("lib", 0) == 0)
        {
          filename = filename.substr(3);
        }
        std::string function_name = "create_" + filename + "_handler";
        RequestHandler *(*create_handler)() = reinterpret_cast<RequestHandler *(*)()>(dlsym(lib_handle, function_name.c_str()));
        if (!create_handler)
        {
          dlclose(lib_handle);
          throw std::runtime_error("Failed to find " + function_name + " function in: " + entry.path().string());
        }
        handlers.emplace_back(create_handler());
      }
    }

    return handlers;
  }

  /**
   * Handle an HTTP request. This function iterates over all loaded request handlers and
   * calls their handle_request method if the request target starts with the handler's endpoint.
   *
   * @param req HTTP request to handle.
   * @return HTTP response.
   */
  http::response<http::string_body> handle_request(http::request<http::string_body> const &req, const std::string &ip_address)
  {
    static std::vector<std::unique_ptr<RequestHandler>> handlers = load_handlers(".");
    http::response<http::string_body> res;
    std::string allowed_methods = "DELETE, GET, OPTIONS, PATCH, POST, PUT";

    std::string origin = req["Origin"].to_string();
    bool is_origin_allowed = (origin.empty() || origin == READER_ALLOWED_ORIGIN || READER_ALLOWED_ORIGIN == "*");

    // Handle CORS preflight request
    if (req.method() == http::verb::options)
    {
      res = {http::status::no_content, req.version()};
      if (is_origin_allowed)
      {
        res.set(http::field::access_control_allow_origin, READER_ALLOWED_ORIGIN);
        res.set(http::field::access_control_allow_methods, allowed_methods);
        res.set(http::field::access_control_allow_headers, "Content-Type, Authorization, Access-Control-Allow-Origin");
        res.set(http::field::access_control_allow_credentials, "true");
      }
      res.set(http::field::connection, "keep-alive");
      return res;
    }

    // Reject requests from disallowed origins
    if (!is_origin_allowed)
    {
      res = {http::status::forbidden, req.version()};
      res.set(http::field::content_type, "text/plain");
      res.body() = "Forbidden: Origin not allowed";
      res.prepare_payload();
      return res;
    }

    for (const auto &handler : handlers)
    {
      if (req.target().starts_with(handler->get_endpoint()))
      {
        res = handler->handle_request(req, ip_address);
        break;
      }
    }

    if (res.result() == http::status::unknown)
    {
      std::cerr << "No handler found for endpoint: " << req.target() << std::endl;
      res = {http::status::not_found, req.version()};
    }

    // Set CORS headers
    if (is_origin_allowed)
    {
      res.set(http::field::access_control_allow_origin, READER_ALLOWED_ORIGIN);
      res.set(http::field::access_control_allow_methods, allowed_methods);
      res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
      res.set(http::field::access_control_allow_credentials, "true");
    }
    res.set(http::field::connection, "keep-alive");
    return res;
  }

  Session::Session(tcp::socket socket) : socket_(std::move(socket)) {}
  void Session::run()
  {
    do_read();
  }

  /**
   * Close the connection with the client.
   */
  void Session::do_close()
  {
    if (closed_)
      return;
    closed_ = true;

    auto self(shared_from_this());
    beast::error_code ec;

    if (socket_.is_open())
    {
      socket_.shutdown(tcp::socket::shutdown_send, ec);

      if (ec && ec != beast::errc::not_connected)
      {
        std::cerr << "Shutdown error: " << ec.message() << std::endl;
      }
    }

    socket_.close(ec);
    if (ec && ec != beast::errc::not_connected)
    {
      std::cerr << "Close error: " << ec.message() << std::endl;
    }
  }

  /**
   * Read a request from the client.
   */
  void Session::do_read()
  {
    if (closed_)
      return;

    buffer_.consume(buffer_.size());
    req_ = {};

    auto self(shared_from_this());

    http::async_read(socket_, buffer_, req_,
                     [this, self](beast::error_code ec, std::size_t)
                     {
                       if (ec == http::error::end_of_stream ||
                           ec == net::error::connection_reset ||
                           ec == net::error::operation_aborted ||
                           ec == net::error::connection_aborted)
                       {
                         return do_close();
                       }

                       if (ec)
                       {
                         std::cerr << "Read error: " << ec.message() << std::endl;
                         return do_close();
                       }

                       boost::asio::ip::address ip_address = socket_.remote_endpoint().address();
                       std::string ip_str = ip_address.to_string();
                       http::response<http::string_body> res = handle_request(req_, ip_str);
                       do_write(std::move(res));
                     });
  }

  /**
   * Write a response to the client.
   * @param res Response to write.
   */
  void Session::do_write(http::response<http::string_body> res)
  {
    if (closed_)
      return;

    auto self(shared_from_this());
    auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));

    if (req_.keep_alive())
    {
      sp->set(http::field::connection, "keep-alive");
    }

    http::async_write(socket_, *sp, [this, self, sp](beast::error_code ec, std::size_t)
                      {
      if (ec)
      {
        std::cerr << "Write error: " << ec.message() << std::endl;
        return do_close();
      }
      if (!req_.keep_alive())
      {
        return do_close();
      }
      if (sp->need_eof())
      {
        return do_close();
      }
      do_read(); });
  }

  Listener::Listener(net::io_context &ioc, tcp::endpoint endpoint) : ioc_(ioc),
                                                                     acceptor_(net::make_strand(ioc))
  {
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
      std::cerr << "Open error: " << ec.message() << std::endl;
      return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
      std::cerr << "Set option error: " << ec.message() << std::endl;
      return;
    }

    acceptor_.set_option(net::socket_base::receive_buffer_size(65536), ec);
    if (ec)
    {
      std::cerr << "Set receive buffer size error: " << ec.message() << std::endl;
      return;
    }

    acceptor_.set_option(net::socket_base::send_buffer_size(65536), ec);
    if (ec)
    {
      std::cerr << "Set send buffer size error: " << ec.message() << std::endl;
      return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
      std::cerr << "Set reuse address error: " << ec.message() << std::endl;
      return;
    }

    acceptor_.set_option(tcp::no_delay(true), ec);
    if (ec)
    {
      std::cerr << "Set no delay error: " << ec.message() << std::endl;
      return;
    }

    acceptor_.bind(endpoint, ec);
    if (ec)
    {
      std::cerr << "Bind error: " << ec.message() << std::endl;
      return;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
    {
      std::cerr << "Listen error: " << ec.message() << std::endl;
      return;
    }

    do_accept();
  }

  /**
   * Start accepting incoming connections.
   */
  void Listener::do_accept()
  {
    acceptor_.async_accept(net::make_strand(ioc_), [this](beast::error_code ec, tcp::socket socket)
                           {
      if (!ec)
      {
        std::make_shared<Session>(std::move(socket))->run();
      }
      do_accept(); });
  }
}