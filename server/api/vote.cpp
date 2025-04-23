#include "api.hpp"

using namespace postgres;
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
   * @param verbose Whether to print messages to stdout.
   * @return JSON of interaction data.
   */
  nlohmann::json select_interaction_data(int annotation_id, bool verbose)
  {
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
        verbose &&std::cerr << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty() || r[0][0].is_null())
      {
        verbose &&std::cout << "Interactions not found" << std::endl;
        return nlohmann::json();
      }

      vote_info = nlohmann::json::parse(r[0][0].as<std::string>());
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
    }
    return vote_info;
  }

  /**
   * Select the interaction type for a specific annotation and user.
   * This is used to determine if the user has already liked or disliked an annotation.
   *
   * @param annotation_id ID of the annotation to select the interaction for.
   * @param user_id ID of the user to select the interaction for.
   * @param verbose Whether to print messages to stdout.
   * @return Interaction type (LIKE or DISLIKE) if found, empty string otherwise.
   */
  std::string select_annotation_interaction_type(int annotation_id, int user_id, bool verbose)
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
        verbose &&std::cerr << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.empty())
      {
        verbose &&std::cout << "Annotation interaction not found" << std::endl;
        return "";
      }
      return r[0][0].as<std::string>();
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
    }
    return "";
  }

  /**
   * Insert a new interaction for a specific annotation and user.
   *
   * @param annotation_id ID of the annotation to insert the interaction for.
   * @param user_id ID of the user to insert the interaction for.
   * @param interaction_type Type of interaction (LIKE or DISLIKE).
   * @param verbose Whether to print messages to stdout.
   * @return true if the interaction was inserted, false otherwise.
   */
  bool insert_interaction(int annotation_id, int user_id, std::string interaction_type, bool verbose)
  {
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
        verbose &&std::cerr << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.affected_rows() == 0)
      {
        verbose &&std::cout << "Failed to insert interaction" << std::endl;
        return false;
      }
      return true;
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
    }
    return false;
  }

  /**
   * Delete an interaction for a specific annotation and user.
   *
   * @param annotation_id ID of the annotation to delete the interaction for.
   * @param user_id ID of the user to delete the interaction for.
   * @param verbose Whether to print messages to stdout.
   * @return true if the interaction was deleted, false otherwise.
   */
  bool delete_interaction(int annotation_id, int user_id, bool verbose)
  {
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
        verbose &&std::cerr << "Error committing transaction: " << e.what() << std::endl;
        throw;
      }

      if (r.affected_rows() == 0)
      {
        verbose &&std::cout << "Interaction not found" << std::endl;
        return false;
      }
      return true;
    }
    catch (const std::exception &e)
    {
      verbose &&std::cerr << "Error executing query: " << e.what() << std::endl;
    }
    catch (...)
    {
      verbose &&std::cerr << "Unknown error while executing query" << std::endl;
    }
    return false;
  }

public:
  VoteHandler(ConnectionPool &connection_pool) : pool(connection_pool)
  {
    pqxx::connection *conn = pool.acquire();
    conn->prepare("select_interaction_data",
                 "SELECT array_to_json(array_agg(row_to_json(t))) "
                 "FROM ("
                 "  SELECT json_build_object("
                 "           'user_id', uai.user_id,"
                 "           'type', uai.type"
                 "         ) as interaction "
                 "  FROM public.\"UserAnnotationInteraction\" uai"
                 "  WHERE uai.annotation_id = $1"
                 ") t");

    conn->prepare("select_annotation_interaction_type",
                 "SELECT type "
                 "FROM public.\"UserAnnotationInteraction\" "
                 "WHERE annotation_id = $1 "
                 "AND user_id = $2");

    conn->prepare("insert_interaction",
                 "INSERT INTO public.\"UserAnnotationInteraction\" ("
                 "annotation_id, user_id, type"
                 ") VALUES ("
                 "$1, $2, $3"
                 ")");

    conn->prepare("delete_interaction",
                 "DELETE FROM public.\"UserAnnotationInteraction\" "
                 "WHERE annotation_id = $1 "
                 "AND user_id = $2");
  }

  std::string get_endpoint() const override
  {
    return "/vote";
  }

  http::response<http::string_body> handle_request(const http::request<http::string_body> &req, const std::string &ip_address)
  {
    if (middleware::rate_limited(ip_address, "/vote", 5))
    {
      return request::make_too_many_requests_response("Too many requests", req);
    }
    if (req.method() == http::verb::get)
    {
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

      nlohmann::json vote_info = select_interaction_data(annotation_id, false);
      if (vote_info.empty())
      {
        return request::make_ok_request_response("No interactions found", req);
      }

      return request::make_json_request_response(vote_info, req);
    }
    else if (req.method() == http::verb::post)
    {
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
      if (!request::validate_session(std::string(session_id), false))
      {
        return request::make_unauthorized_response("Invalid session ID", req);
      }

      int real_user_id = request::get_user_id_from_session(std::string(session_id), false);
      if (real_user_id == -1)
      {
        return request::make_bad_request_response("User not found", req);
      }
      if (real_user_id != user_id)
      {
        return request::make_bad_request_response("User ID mismatch. This incident has been reported", req);
      }
      if (!middleware::user_accepted_policy(user_id, false))
      {
        return request::make_unauthorized_response("User has not accepted the privacy policy", req);
      }

      std::string interaction_type = select_annotation_interaction_type(annotation_id, user_id, false);
      std::string new_interaction_type = interaction == 1 ? "LIKE" : "DISLIKE";

      if (interaction_type.empty())
      {
        // Insert the Interaction
        if (!insert_interaction(annotation_id, user_id, new_interaction_type, false))
        {
          return request::make_bad_request_response("Failed to insert interaction", req);
        }
        return request::make_ok_request_response("Interaction inserted", req);
      }

      if (!delete_interaction(annotation_id, user_id, false))
      {
        return request::make_bad_request_response("Failed to delete interaction", req);
      }
      if (interaction_type == new_interaction_type)
      {
        return request::make_ok_request_response("Interaction removed", req);
      }

      // Insert the Interaction
      if (!insert_interaction(annotation_id, user_id, new_interaction_type, false))
      {
        return request::make_bad_request_response("Failed to insert interaction", req);
      }

      return request::make_ok_request_response("Interaction inserted", req);
    }
    else
    {
      return request::make_bad_request_response("Invalid method", req);
    }
  }
};

extern "C" RequestHandler *create_vote_handler()
{
  return new VoteHandler(get_connection_pool());
}