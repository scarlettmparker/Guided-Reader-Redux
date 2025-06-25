#include "api.hpp"

using namespace postgres;
using namespace utils;

class TextHandler : public RequestHandler
{
private:
  ConnectionPool &pool;

  /**
   * Select annotation positions for a text. This will return the start and end
   * positions of each annotation in the text along with its ID.
   *
   * @param text_object_id ID of the text object to select annotations for.
   * @param language Language of the text object to select annotations for.
   */
  nlohmann::json select_annotations(int text_object_id, std::string language)
  {
    Logger::instance().debug("Selecting annotations for text_object_id=" + std::to_string(text_object_id) + ", language=" + language);
    nlohmann::json text_data = nlohmann::json::array();

    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result text_id_result = txn.exec_prepared(
          "select_text_id",
          std::to_string(text_object_id), language);

      if (text_id_result.empty())
      {
        return text_data;
      }

      int text_id = text_id_result[0][0].as<int>();

      pqxx::result r = txn.exec_prepared(
          "select_annotations",
          std::to_string(text_id));

      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        utils::Logger::instance().error(std::string("Error committing transaction: ") + e.what());
        throw;
      }

      if (r.empty())
      {
        return text_data;
      }

      if (r[0][0].is_null())
      {
      }
      else
      {
        text_data = nlohmann::json::parse(r[0][0].as<std::string>());
      }
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while executing query");
    }
    return text_data;
  }

  /**
   * Select text data from the database. This will return the text and language of a text object.
   * @param text__object_id ID of the text object to select.
   * @param language Language of the text object to select.
   * @return JSON of text data.
   */
  nlohmann::json select_text_data(int text_object_id, std::string language)
  {
    Logger::instance().debug("Selecting text data for text_object_id=" + std::to_string(text_object_id) + ", language=" + language);
    nlohmann::json text_data = nlohmann::json::array();
    std::string cache_key = "text:" + std::to_string(text_object_id) + ":" + language;
    sw::redis::Redis &redis = Redis::get_instance();

    try
    {
      std::optional<std::string> cache_result = redis.get(cache_key);

      if (cache_result)
      {
        return nlohmann::json::parse(*cache_result);
      }

      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_text_details",
          std::to_string(text_object_id), language);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        throw;
      }

      if (r.empty())
      {
        return text_data;
      }

      text_data = nlohmann::json::parse(r[0][0].as<std::string>());
      redis.set(cache_key, text_data.dump(), std::chrono::seconds(300)); // 5 minutes
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
    }
    return text_data;
  }

  /**
   * Select brief text data from the database. This will return the text title
   * and author of a text object, along with some other metadata.
   *
   * @param text_object_id ID of the text object to select.
   * @param language Language of the text object to select.
   * @return JSON of brief text data.
   */
  nlohmann::json select_text_brief(int text_object_id, std::string language)
  {
    Logger::instance().debug("Selecting text brief for text_object_id=" + std::to_string(text_object_id) + ", language=" + language);
    nlohmann::json text_data = nlohmann::json::array();
    std::string cache_key = "text:" + std::to_string(text_object_id) + ":" + language + ":brief";
    sw::redis::Redis &redis = Redis::get_instance();

    try
    {
      std::optional<std::string> cache_result = redis.get(cache_key);

      if (cache_result)
      {
        return nlohmann::json::parse(*cache_result);
      }

      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_text_brief",
          std::to_string(text_object_id), language);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        throw;
      }

      if (r.empty())
      {
        return text_data;
      }

      text_data = nlohmann::json::parse(r[0][0].as<std::string>());
      redis.set(cache_key, text_data.dump(), std::chrono::seconds(300)); // 5 minutes
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
    }
    return text_data;
  }

public:
  TextHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
  }

  std::string get_endpoint() const override
  {
    return "/text";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    Logger::instance().info("Text endpoint called: " + std::string(req.method_string()));
    if (middleware::rate_limited(ip_address, "/text", 20))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
      Logger::instance().debug("GET text requested");
      /**
       * GET text details.
       */
      std::optional<std::string> text_object_id_param = request::parse_from_request(req, "text_object_id");
      std::optional<std::string> language_param = request::parse_from_request(req, "language");
      std::optional<std::string> type_param = request::parse_from_request(req, "type");

      if (!text_object_id_param.has_value() || !language_param.has_value())
      {
        return request::make_bad_request_response("Missing parameters text_object_id | language", req);
      }

      int text_object_id;
      std::string language = language_param.value();

      try
      {
        text_object_id = std::stoi(text_object_id_param.value());
      }
      catch (const std::invalid_argument &)
      {
        return request::make_bad_request_response("Invalid numeric value for text_object_id", req);
      }
      catch (const std::out_of_range &)
      {
        return request::make_bad_request_response("Number out of range for text_object_id", req);
      }

      if (!language_param)
      {
        return request::make_bad_request_response("Missing language parameter", req);
      }

      if (type_param.has_value() && type_param.value() == "brief")
      {
        nlohmann::json brief_text_data = select_text_brief(text_object_id, language);
        return request::make_json_request_response(brief_text_data, req);
      }

      if (type_param.has_value() && type_param.value() == "annotations")
      {
        nlohmann::json annotation_data = select_annotations(text_object_id, language);
        return request::make_json_request_response(annotation_data, req);
      }

      nlohmann::json text_info = select_text_data(text_object_id, language);
      if (text_info.empty())
      {
        Logger::instance().info("No text found for text_object_id=" + std::to_string(text_object_id));
        return request::make_bad_request_response("No text found", req);
      }

      if (type_param.has_value() && type_param.value() == "all")
      {
        nlohmann::json annotations = select_annotations(text_object_id, language);
        text_info[0]["annotations"] = annotations;
      }

      Logger::instance().info("Text data returned for text_object_id=" + std::to_string(text_object_id));
      return request::make_json_request_response(text_info, req);
    }
    else
    {
      Logger::instance().info("Invalid method for text endpoint");
      return request::make_bad_request_response("Invalid request method", req);
    }
  }
};

extern "C" RequestHandler *create_text_handler()
{
  return new TextHandler(get_connection_pool());
}