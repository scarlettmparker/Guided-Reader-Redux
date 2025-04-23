#ifndef REDIS_HPP
#define REDIS_HPP

#include <sw/redis++/redis++.h>
#include <iostream>
#include <memory>
#include <string>

#include "config.h"

class Redis
{
private:
  static std::unique_ptr<sw::redis::Redis> instance_;
  Redis() = delete;

public:
  static void init_connection();
  static sw::redis::Redis &get_instance();
};

#endif