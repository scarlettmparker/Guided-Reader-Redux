#include "api.hpp"

using namespace postgres;
using namespace utils;

class VoteHandler : public RequestHandler
{
private:
  ConnectionPool &pool;

  /**
   * Select interaction data for a specific annotation. This will return a list
   * of interactions (either LIKE or DISLIKE) for a given annotation, along with
   * the user ID that performed the interaction.
   *
   * @param annotation_id ID of the annotation to select interactions for.
   * @return JSON of interaction data.
   */
  nlohmann::json select_interaction_data(int annotation_id)
  {
    Logger::instance().debug("Selecting interaction data for annotation_id=" + std::to_string(annotation_id));
    nlohmann::json vote_info = nlohmann::json::array();
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_interaction_data",
          annotation_id);
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
        utils::Logger::instance().debug("Interactions not found");
        return nlohmann::json();
      }
      vote_info = nlohmann::json::parse(r[0][0].as<std::string>());
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while executing query");
    }
    return vote_info;
  }

  /**
   * Select the interaction type for a specific annotation and user.
   * This is used to determine if the user has already liked or disliked an annotation.
   *
   * @param annotation_id ID of the annotation to select the interaction for.
   * @param user_id ID of the user to select the interaction for.
   * @return Interaction type (LIKE or DISLIKE) if found, empty string otherwise.
   */
  std::string select_annotation_interaction_type(int annotation_id, int user_id)
  {
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "select_annotation_interaction_type",
          annotation_id, user_id);
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
        utils::Logger::instance().debug("Annotation interaction not found");
        return "";
      }
      return r[0][0].as<std::string>();
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while executing query");
    }
    return "";
  }

  /**
   * Insert a new interaction for a specific annotation and user.
   *
   * @param annotation_id ID of the annotation to insert the interaction for.
   * @param user_id ID of the user to insert the interaction for.
   * @param interaction_type Type of interaction (LIKE or DISLIKE).
   * @return true if the interaction was inserted, false otherwise.
   */
  bool insert_interaction(int annotation_id, int user_id, std::string interaction_type)
  {
    Logger::instance().debug("Inserting interaction for annotation_id=" + std::to_string(annotation_id) + ", user_id=" + std::to_string(user_id));
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "insert_interaction",
          annotation_id, user_id, interaction_type);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        utils::Logger::instance().error(std::string("Error committing transaction: ") + e.what());
        throw;
      }
      if (r.affected_rows() == 0)
      {
        utils::Logger::instance().error("Failed to insert interaction");
        return false;
      }
      return true;
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while executing query");
    }
    return false;
  }

  /**
   * Delete an interaction for a specific annotation and user.
   *
   * @param annotation_id ID of the annotation to delete the interaction for.
   * @param user_id ID of the user to delete the interaction for.
   * @return true if the interaction was deleted, false otherwise.
   */
  bool delete_interaction(int annotation_id, int user_id)
  {
    Logger::instance().debug("Deleting interaction for annotation_id=" + std::to_string(annotation_id) + ", user_id=" + std::to_string(user_id));
    try
    {
      pqxx::work &txn = request::begin_transaction(pool);
      pqxx::result r = txn.exec_prepared(
          "delete_interaction",
          annotation_id, user_id);
      try
      {
        txn.commit();
      }
      catch (const std::exception &e)
      {
        utils::Logger::instance().error(std::string("Error committing transaction: ") + e.what());
        throw;
      }
      if (r.affected_rows() == 0)
      {
        utils::Logger::instance().debug("Interaction not found");
        return false;
      }
      return true;
    }
    catch (const std::exception &e)
    {
      Logger::instance().error(std::string("Error executing query: ") + e.what());
    }
    catch (...)
    {
      utils::Logger::instance().error("Unknown error while executing query");
    }
    return false;
  }

public:
  VoteHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
  }

  std::string get_endpoint() const override
  {
    return "/vote";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    Logger::instance().info("Vote endpoint called: " + std::string(req.method_string()));
    if (middleware::rate_limited(ip_address, "/vote", 5))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
      Logger::instance().debug("GET vote details requested");
      /**
       * GET vote details for a specific annotation.
       */
      std::optional<std::string> annotation_id_param = request::parse_from_request(req, "annotation_id");
      if (!annotation_id_param)
      {
        return request::make_bad_request_response("Missing parameter annotation_id", req);
      }

      int annotation_id;
      try
      {
        annotation_id = std::stoi(annotation_id_param.value());
      }
      catch (const std::invalid_argument &)
      {
        return request::make_bad_request_response("Invalid numeric value for annotation_id", req);
      }
      catch (const std::out_of_range &)
      {
        return request::make_bad_request_response("Number out of range for annotation_id", req);
      }

      nlohmann::json vote_info = select_interaction_data(annotation_id);
      if (vote_info.empty())
      {
        Logger::instance().info("No interactions found for annotation_id=" + std::to_string(annotation_id));
        return request::make_ok_request_response("No interactions found", req);
      }

      Logger::instance().info("Vote data returned for annotation_id=" + std::to_string(annotation_id));
      return request::make_json_request_response(vote_info, req);
    }
    else if (req.method() == http::verb::post)
    {
      Logger::instance().debug("POST vote requested");
      /**
       * POST a new vote.
       */
      nlohmann::json json_request;
      try
      {
        json_request = nlohmann::json::parse(req.body());
      }
      catch (const nlohmann::json::parse_error &e)
      {
        return request::make_bad_request_response("Invalid JSON", req);
      }
      if (!json_request.contains("user_id") || !json_request.contains("annotation_id") || !json_request.contains("interaction"))
      {
        return request::make_bad_request_response("Missing parameters user_id | annotation_id | interaction", req);
      }

      int user_id, annotation_id, interaction;
      try
      {
        user_id = json_request["user_id"].get<int>();
        annotation_id = json_request["annotation_id"].get<int>();
        interaction = json_request["interaction"].get<int>();
      }
      catch (const nlohmann::json::type_error &e)
      {
        return request::make_bad_request_response("Invalid parameter types", req);
      }
      if (interaction != 1 && interaction != -1)
      {
        return request::make_bad_request_response("Invalid interaction value", req);
      }

      // Ensure the user is authenticated
      std::string_view session_id = request::get_session_id_from_cookie(req);
      if (session_id.empty())
      {
        return request::make_unauthorized_response("Session ID not found", req);
      }
      if (!request::validate_session(std::string(session_id)))
      {
        return request::make_unauthorized_response("Invalid session ID", req);
      }

      int real_user_id = request::get_user_id_from_session(std::string(session_id));
      if (real_user_id == -1)
      {
        return request::make_bad_request_response("User not found", req);
      }
      if (real_user_id != user_id)
      {
        return request::make_bad_request_response("User ID mismatch. This incident has been reported", req);
      }
      if (!middleware::user_accepted_policy(user_id))
      {
        return request::make_unauthorized_response("User has not accepted the privacy policy", req);
      }

      std::string interaction_type = select_annotation_interaction_type(annotation_id, user_id);
      std::string new_interaction_type = interaction == 1 ? "LIKE" : "DISLIKE";

      if (interaction_type.empty())
      {
        // Insert the Interaction
        if (!insert_interaction(annotation_id, user_id, new_interaction_type))
        {
          Logger::instance().error("Failed to insert interaction for annotation_id=" + std::to_string(annotation_id));
          return request::make_bad_request_response("Failed to insert interaction", req);
        }
        Logger::instance().info("Interaction inserted for annotation_id=" + std::to_string(annotation_id));
        return request::make_ok_request_response("Interaction inserted", req);
      }

      if (!delete_interaction(annotation_id, user_id))
      {
        return request::make_bad_request_response("Failed to delete interaction", req);
      }
      if (interaction_type == new_interaction_type)
      {
        return request::make_ok_request_response("Interaction removed", req);
      }

      // Insert the Interaction
      if (!insert_interaction(annotation_id, user_id, new_interaction_type))
      {
        return request::make_bad_request_response("Failed to insert interaction", req);
      }

      return request::make_ok_request_response("Interaction inserted", req);
    }
    else
    {
      Logger::instance().info("Invalid method for vote endpoint");
      return request::make_bad_request_response("Invalid method", req);
    }
  }
};

extern "C" RequestHandler *create_vote_handler()
{
  return new VoteHandler(get_connection_pool());
}