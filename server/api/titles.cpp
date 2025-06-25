#include "api.hpp"

using namespace postgres;
using namespace utils;

class TitlesHandler : public RequestHandler
{
private:
  ConnectionPool &pool;

  /**
   * Select title data from the database. This will return a list of text tiles
   * along with their level and group ID. This is used for lazy loading on the
   * front end as it takes page and page size params and the ID can be used
   * later on to fetch more detailed information.
   *
   * @param page Page number to fetch.
   * @param page_size Number of items to fetch.
   * @param sort Sort order to use.
   * @return JSON of title data.
   */
  nlohmann::json select_title_data(int page, int page_size, int sort)
  {
    Logger::instance().debug("Selecting title data for page=" + std::to_string(page) + ", page_size=" + std::to_string(page_size) + ", sort=" + std::to_string(sort));
    std::string sort_query;
    nlohmann::json title_info = nlohmann::json::array();

    std::string cache_key = "titles:" + std::to_string(page) + ":" +
                            std::to_string(page_size) + ":" + std::to_string(sort);
    sw::redis::Redis &redis = Redis::get_instance();

    if (sort == 0)
    {
      sort_query = "ORDER BY id";
    }

    try
    {
      std::optional<std::string> cache_result = redis.get(cache_key);

      if (cache_result)
      {
        Logger::instance().debug("Cache hit for " + cache_key);
        return nlohmann::json::parse(*cache_result);
      }

      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_titles",
          std::to_string(page_size), std::to_string(page * page_size));
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        utils::Logger::instance().error(std::string("Error committing transaction: ") + e.what());
        throw;
      }

      if (r.empty() || r[0][0].is_null())
      {
        Logger::instance().debug("No titles found");
        return title_info;
      }

      title_info = nlohmann::json::parse(r[0][0].as<std::string>());
      redis.set(cache_key, title_info.dump(), std::chrono::seconds(300)); // 5 minutes
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while executing query");
    }
    return title_info;
  }

public:
  TitlesHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
  }

  std::string get_endpoint() const override
  {
    return "/titles";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    Logger::instance().info("Titles endpoint called: " + std::string(req.method_string()));
    if (middleware::rate_limited(ip_address, "/titles", 50))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
      Logger::instance().debug("GET titles requested");
      /**
       * GET text titles
       */
      std::optional<std::string> page_param = request::parse_from_request(req, "page");
      std::optional<std::string> page_size_param = request::parse_from_request(req, "page_size");
      std::optional<std::string> sort_param = request::parse_from_request(req, "sort");

      if (!page_param || !page_size_param)
      {
        return request::make_bad_request_response("Missing parameters page | page_size", req);
      }

      int page, page_size, sort = 0;

      try
      {
        page = std::stoi(page_param.value());
        page_size = std::stoi(page_size_param.value());

        if (sort_param)
        {
          sort = std::stoi(sort_param.value());
        }
      }
      catch (const std::invalid_argument &)
      {
        return request::make_bad_request_response("Invalid numeric value for page | page_size | sort", req);
      }
      catch (const std::out_of_range &)
      {
        return request::make_bad_request_response("Number out of range for page | page_size | sort", req);
      }

      nlohmann::json title_info = select_title_data(page, page_size, sort);
      if (title_info.empty())
      {
        Logger::instance().info("No titles found for page=" + std::to_string(page));
        return request::make_bad_request_response("No titles found", req);
      }
      Logger::instance().info("Titles data returned for page=" + std::to_string(page));

      return request::make_json_request_response(title_info, req);
    }
    else
    {
      Logger::instance().info("Invalid method for titles endpoint");
      return request::make_bad_request_response("Invalid request method", req);
    }
  }
};

extern "C" RequestHandler *create_titles_handler()
{
  return new TitlesHandler(get_connection_pool());
}