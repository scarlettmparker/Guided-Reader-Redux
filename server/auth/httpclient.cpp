#include "httpclient.hpp"

namespace httpclient
{
  HTTPClient::HTTPClient(const std::string &host, const std::string &port, bool use_ssl)
      : host_(host), port_(port), use_ssl_(use_ssl) {}

  /**
   * Make a GET request to a given target.
   * @param target Target to make the request to.
   * @return Response from the server.
   */
  std::string HTTPClient::get(const std::string &target)
  {
    return do_request(beast::http::verb::get, target);
  }

  /**
   * Make a POST request to a given target with a body.
   * @param target Target to make the request to.
   * @param body Body to send with the request.
   * @return Response from the server.
   */
  std::string HTTPClient::post(const std::string &target, const std::string &body)
  {
    return do_request(beast::http::verb::post, target, body);
  }

  /**
   * Make a PUT request to a given target with a body.
   * @param target Target to make the request to.
   * @param body Body to send with the request.
   * @return Response from the server.
   */
  std::string HTTPClient::put(const std::string &target, const std::string &body)
  {
    return do_request(beast::http::verb::put, target, body);
  }

  /**
   * Make a PATCH request to a given target with a body.
   * @param target Target to make the request to.
   * @param body Body to send with the request.
   * @return Response from the server.
   */
  std::string HTTPClient::patch(const std::string &target, const std::string &body)
  {
    return do_request(beast::http::verb::patch, target, body);
  }

  /**
   * Make a DELETE request to a given target.
   * @param target Target to make the request to.
   * @return Response from the server.
   * @return Response from the server.
   */
  std::string HTTPClient::delete_(const std::string &target)
  {
    return do_request(beast::http::verb::delete_, target);
  }

  /**
   * Set the content type for the request. Used for specifying the type of data
   * being sent in the request body. For example, 'application/x-www-form-urlencoded'
   *
   * @param content_type Content type to set.
   */
  void HTTPClient::set_content_type(const std::string &content_type)
  {
    content_type_ = content_type;
  }

  /**
   * Set the authorization header for the request. Used for including
   * Discord auth tokens in API requests for Discord account authentication.
   *
   * @param auth_header Authorization header to set.
   */
  void HTTPClient::set_authorization(const std::string &auth_header)
  {
    auth_header_ = auth_header;
  }

  /**
   * @brief Performs an HTTP request using the specified method, target, and body.
   *
   * This function sets up a TCP connection, optionally performs an SSL handshake,
   * constructs an HTTP request, sends it to the server, and reads the response.
   *
   * @param method HTTP method to use for the request (e.g., GET, POST).
   * @param target Target URI for the request (e.g., "/api/resource").
   * @param body Body of the request. If empty, no body is sent.
   * @return Body of the HTTP response received from the server.
   *
   * @details
   * - The function resolves the host and port using a TCP resolver.
   * - It establishes a TCP connection and optionally performs an SSL handshake if `use_ssl_` is true.
   * - An HTTP request is created with the specified method and target, and the "Host" and "User-Agent" headers are set.
   * - If the body is not empty, it is included in the request and the payload is prepared.
   * - The request is sent to the server, and the response is read into a buffer.
   * - The function attempts to gracefully shut down the SSL/TCP connection.
   * - If an error occurs during shutdown, it is ignored.
   * - The body of the HTTP response is returned as a string.
   */
  std::string HTTPClient::do_request(
      beast::http::verb method, const std::string &target, const std::string &body)
  {
    try
    {
      // Resolve the host and port and set up the stream
      tcp::resolver resolver(ioc_);
      auto const results = resolver.resolve(host_, port_);
      beast::ssl_stream<beast::tcp_stream> stream(ioc_, ctx_);
      beast::get_lowest_layer(stream).connect(results);

      if (use_ssl_)
      {
        stream.handshake(ssl::stream_base::client);
      }

      // Create the request
      http::request<http::string_body> req{method, target, 11};
      req.set(http::field::host, host_);
      req.set(http::field::user_agent, "guided_reader");

      if (!auth_header_.empty())
      {
        req.set(http::field::authorization, auth_header_);
      }

      if (!body.empty())
      {
        req.body() = body;
        req.set(http::field::content_type, content_type_);
        req.prepare_payload();
      }

      // Send the request and read the response
      http::write(stream, req);
      beast::flat_buffer buffer;
      http::response<http::string_body> res;
      http::read(stream, buffer, res);

      beast::error_code ec;
      stream.shutdown(ec);
      return res.body();
    }
    catch (const std::exception &e)
    {
      std::cerr << "HTTP request error: " << e.what() << std::endl;
      return "";
    }
  }
}