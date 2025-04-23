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

    pqxx::work txn(*c);

    // Text queries
    txn.conn().prepare("select_text_id",
                       "SELECT id "
                       "FROM public.\"Text\" "
                       "WHERE text_object_id = $1 "
                       "AND language = $2");

    txn.conn().prepare("select_annotations",
                       "SELECT array_to_json(array_agg(row_to_json(t))) "
                       "FROM ("
                       "  SELECT id::integer,"
                       "         start::integer,"
                       "         \"end\"::integer,"
                       "         text_id::integer"
                       "  FROM public.\"Annotation\" "
                       "  WHERE text_id = $1"
                       ") t");

    txn.conn().prepare("select_text_details",
                       "SELECT array_to_json(array_agg(row_to_json(t))) "
                       "FROM ("
                       "  SELECT id::integer,"
                       "         text::text,"
                       "         language::text,"
                       "         text_object_id::integer,"
                       "         (SELECT row_to_json(a) "
                       "          FROM ("
                       "            SELECT id, audio_file, vtt_file, submission_group, submission_url "
                       "            FROM public.\"Audio\" "
                       "            WHERE id = t.audio_id"
                       "          ) a"
                       "         ) as audio"
                       "  FROM public.\"Text\" t"
                       "  WHERE text_object_id = $1"
                       "  AND language = $2"
                       ") t");

    txn.conn().prepare("select_text_brief",
                       "SELECT array_to_json(array_agg(row_to_json(t))) "
                       "FROM ("
                       "  SELECT t.id::integer,"
                       "         tobj.title::text,"
                       "         tobj.brief::text,"
                       "         tobj.level::text,"
                       "         t.audio_id::integer,"
                       "         json_build_object("
                       "           'id', tg.id,"
                       "           'group_name', tg.group_name,"
                       "           'group_url', tg.group_url"
                       "         ) as \"group\","
                       "         CASE WHEN t.author_id IS NOT NULL THEN json_build_object("
                       "           'id', u.id,"
                       "           'username', u.username,"
                       "           'discord_id', u.discord_id,"
                       "           'avatar', u.avatar,"
                       "           'nickname', u.nickname,"
                       "           'discord_status', u.discord_status"
                       "         ) END as author,"
                       "         (SELECT array_agg(language) FROM public.\"Text\" WHERE text_object_id = t.text_object_id) as languages"
                       "  FROM public.\"Text\" t"
                       "  LEFT JOIN public.\"TextObject\" tobj ON t.text_object_id = tobj.id"
                       "  LEFT JOIN public.\"TextGroup\" tg ON tobj.group_id = tg.id"
                       "  LEFT JOIN public.\"User\" u ON t.author_id = u.id"
                       "  WHERE t.text_object_id = $1"
                       "  AND t.language = $2"
                       ") t");

    // Title queries
    txn.conn().prepare("select_titles",
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

    // User queries
    txn.conn().prepare("select_user_id",
                       "SELECT id "
                       "FROM public.\"User\" "
                       "WHERE username = $1 "
                       "LIMIT 1");

    txn.conn().prepare("select_email",
                       "SELECT email "
                       "FROM public.\"User\" "
                       "WHERE email = $1 "
                       "LIMIT 1");

    txn.conn().prepare("select_user_data_by_id",
                       "SELECT row_to_json(t) "
                       "FROM ("
                       "  SELECT id, username, discord_id, avatar, nickname, accepted_policy "
                       "  FROM public.\"User\" "
                       "  WHERE id = $1 "
                       "  LIMIT 1"
                       ") t");

    txn.conn().prepare("select_username_by_id",
                       "SELECT username "
                       "FROM public.\"User\" "
                       "WHERE id = $1 "
                       "LIMIT 1");

    txn.conn().prepare("select_user_password",
                       "SELECT password "
                       "FROM public.\"User\" "
                       "WHERE username = $1 "
                       "LIMIT 1");

    txn.conn().prepare("select_accepted_policy",
                       "SELECT accepted_policy "
                       "FROM public.\"User\" "
                       "WHERE id = $1 "
                       "LIMIT 1");

    txn.conn().prepare("set_accepted_policy",
                       "UPDATE public.\"User\" "
                       "SET accepted_policy = $2 "
                       "WHERE id = $1");

    txn.conn().prepare("insert_user",
                       "INSERT INTO public.\"User\" ("
                       "username, email, password, levels, discord_id, account_creation_date, "
                       "avatar, nickname"
                       ") VALUES ("
                       "$1, $2, $3, '{-1}', '-1', $4, '-1', $1"
                       ")");

    txn.conn().prepare("update_user_roles",
                       "UPDATE public.\"User\" "
                       "SET levels = $2 "
                       "WHERE id = $1");

    txn.conn().prepare("update_user_data",
                       "UPDATE public.\"User\" "
                       "SET avatar = $2, nickname = $3 "
                       "WHERE id = $1");

    // Discord user queries
    txn.conn().prepare("select_user_id_by_discord_id",
                       "SELECT id "
                       "FROM public.\"User\" "
                       "WHERE discord_id = $1 "
                       "LIMIT 1");

    txn.conn().prepare("register_with_discord",
                       "INSERT INTO public.\"User\" ("
                       "discord_id, username, avatar, account_creation_date"
                       ") VALUES ("
                       "$1, $2, $3, $4"
                       ")");

    txn.conn().prepare("link_user_to_discord",
                       "UPDATE public.\"User\" "
                       "SET discord_id = $2 "
                       "WHERE id = $1");

    txn.conn().prepare("validate_discord_status",
                       "UPDATE public.\"User\" "
                       "SET discord_status = true "
                       "WHERE id = $1");

    txn.conn().prepare("invalidate_discord_status",
                       "UPDATE public.\"User\" "
                       "SET discord_status = false "
                       "WHERE id = $1");

    // Profile queries
    txn.conn().prepare("select_profile_data",
                       "SELECT array_to_json(array_agg(row_to_json(t))) "
                       "FROM ("
                       "  SELECT json_build_object("
                       "           'id', u.id,"
                       "           'username', u.username,"
                       "           'discord_id', u.discord_id,"
                       "           'avatar', u.avatar,"
                       "           'nickname', u.nickname,"
                       "           'discord_status', u.discord_status"
                       "         ) as user,"
                       "         u.levels,"
                       "         COUNT(DISTINCT a.id) as annotation_count,"
                       "         COUNT(DISTINCT CASE WHEN uai.type = 'LIKE' THEN uai.id END) as like_count,"
                       "         COUNT(DISTINCT CASE WHEN uai.type = 'DISLIKE' THEN uai.id END) as dislike_count"
                       "  FROM public.\"User\" u"
                       "  LEFT JOIN public.\"Annotation\" a ON a.user_id = u.id"
                       "  LEFT JOIN public.\"UserAnnotationInteraction\" uai ON uai.user_id = u.id"
                       "  WHERE u.id = $1"
                       "  GROUP BY u.id, u.username, u.discord_id, u.avatar, u.nickname, u.discord_status, u.levels"
                       ") t");

    // Annotation queries
    txn.conn().prepare("select_annotation_data",
                       "SELECT array_to_json(array_agg(row_to_json(t))) "
                       "FROM ("
                       "  SELECT json_build_object("
                       "           'id', a.id::integer,"
                       "           'start', a.start,"
                       "           'end', a.\"end\","
                       "           'text_id', a.text_id"
                       "         ) as annotation,"
                       "         a.description::text,"
                       "         COALESCE(SUM(CASE WHEN uai.type = 'LIKE' THEN 1 ELSE 0 END), 0) as likes,"
                       "         COALESCE(SUM(CASE WHEN uai.type = 'DISLIKE' THEN 1 ELSE 0 END), 0) as dislikes,"
                       "         a.created_at::integer,"
                       "         json_build_object("
                       "           'id', u.id,"
                       "           'username', u.username,"
                       "           'discord_id', u.discord_id,"
                       "           'avatar', u.avatar,"
                       "           'discord_status', u.discord_status"
                       "         ) as author "
                       "  FROM public.\"Annotation\" a"
                       "  LEFT JOIN public.\"User\" u ON a.user_id = u.id"
                       "  LEFT JOIN public.\"UserAnnotationInteraction\" uai ON a.id = uai.annotation_id"
                       "  WHERE a.text_id = $1 "
                       "  AND a.start >= $2 "
                       "  AND a.\"end\" <= $3"
                       "  GROUP BY a.id, a.start, a.\"end\", a.text_id, a.description,"
                       "  a.created_at, u.id, u.username, u.discord_id, u.discord_status, u.avatar"
                       ") t");

    txn.conn().prepare("select_annotation_ranges",
                       "SELECT UNNEST(array_agg(start::integer)) as range_start, "
                       "UNNEST(array_agg(\"end\"::integer)) as range_end "
                       "FROM public.\"Annotation\" "
                       "WHERE text_id = $1");

    txn.conn().prepare("select_author_id_by_annotation",
                       "SELECT user_id "
                       "FROM public.\"Annotation\" "
                       "WHERE id = $1");

    txn.conn().prepare("insert_annotation",
                       "INSERT INTO public.\"Annotation\" ("
                       "text_id, user_id, start, \"end\", description, created_at"
                       ") VALUES ("
                       "$1, $2, $3, $4, $5, $6"
                       ")");

    txn.conn().prepare("update_annotation",
                       "UPDATE public.\"Annotation\" "
                       "SET description = $1 "
                       "WHERE id = $2");

    txn.conn().prepare("delete_annotation_interactions",
                       "DELETE FROM public.\"UserAnnotationInteraction\" "
                       "WHERE annotation_id = $1");

    txn.conn().prepare("delete_annotation",
                       "WITH deleted_interactions AS ("
                       "  DELETE FROM public.\"UserAnnotationInteraction\" "
                       "  WHERE annotation_id = $1"
                       ")"
                       "DELETE FROM public.\"Annotation\" "
                       "WHERE id = $1");

    // ... user annotation interaction queries ...
    txn.conn().prepare("select_interaction_data",
                       "SELECT array_to_json(array_agg(row_to_json(t))) "
                       "FROM ("
                       "  SELECT json_build_object("
                       "           'user_id', uai.user_id,"
                       "           'type', uai.type"
                       "         ) as interaction "
                       "  FROM public.\"UserAnnotationInteraction\" uai"
                       "  WHERE uai.annotation_id = $1"
                       ") t");

    txn.conn().prepare("select_annotation_interaction_type",
                       "SELECT type "
                       "FROM public.\"UserAnnotationInteraction\" "
                       "WHERE annotation_id = $1 "
                       "AND user_id = $2");

    txn.conn().prepare("insert_interaction",
                       "INSERT INTO public.\"UserAnnotationInteraction\" ("
                       "annotation_id, user_id, type"
                       ") VALUES ("
                       "$1, $2, $3"
                       ")");

    txn.conn().prepare("delete_interaction",
                       "DELETE FROM public.\"UserAnnotationInteraction\" "
                       "WHERE annotation_id = $1 "
                       "AND user_id = $2");

    txn.commit();

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
