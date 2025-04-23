#include "api.hpp"

using namespace postgres;
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
   * @param verbose Whether to print messages to stdout.
   * @return JSON of title data.
   */
  nlohmann::json select_title_data(int page, int page_size, int sort, bool verbose)
  {
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
        verbose &&std::cout << "Cache hit for " << cache_key << std::endl;
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
        verbose &&std::cerr << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty() || r[0][0].is_null())
      {
        verbose &&std::cout << "No titles found" << std::endl;
        return title_info;
      }

      title_info = nlohmann::json::parse(r[0][0].as<std::string>());
      redis.set(cache_key, title_info.dump(), std::chrono::seconds(300)); // 5 minutes
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
    }
    return title_info;
  }

public:
  TitlesHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
    pqxx::connection *conn = pool.acquire();
    conn->prepare("select_titles",
                 "SELECT array_to_json(array_agg(row_to_json(t))) "
                 "FROM ("
                 "  SELECT id::integer,"
                 "         title::text,"
                 "         level::text,"
                 "         group_id::integer "
                 "  FROM public.\"TextObject\" "
                 "  WHERE id > $2 "
                 "  ORDER BY id "
                 "  LIMIT $1"
                 ") t");
  }

  std::string get_endpoint() const override
  {
    return "/titles";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    if (middleware::rate_limited(ip_address, "/titles", 50))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
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

      nlohmann::json title_info = select_title_data(page, page_size, sort, false);
      if (title_info.empty())
      {
        return request::make_bad_request_response("No titles found", req);
      }

      return request::make_json_request_response(title_info, req);
    }
    else
    {
      return request::make_bad_request_response("Invalid request method", req);
    }
  }
};

extern "C" RequestHandler *create_titles_handler()
{
  return new TitlesHandler(get_connection_pool());
}