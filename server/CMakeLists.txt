cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project(ReaderServer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall -Wextra")

# Use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# ───────────────────────────────────────────────────────────────
# Load environment variables
# ───────────────────────────────────────────────────────────────
include(${CMAKE_CURRENT_SOURCE_DIR}/env/load_env.cmake)

set(ENV_FILE ${CMAKE_CURRENT_SOURCE_DIR}/env/.env)
check_env_file_changed(${ENV_FILE} ENV_CHANGED)
if(ENV_CHANGED)
  message("Environment file changed, forcing config.h regeneration")
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/env/config.h)
endif()

configure_file(${ENV_FILE} ${CMAKE_CURRENT_BINARY_DIR}/env_file_dependency COPYONLY)
load_env_file(${ENV_FILE})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/env/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/env/config.h)
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/env
  ${Boost_INCLUDE_DIRS}
  ${LIBPQ_INCLUDE_DIRS}
)

# ───────────────────────────────────────────────────────────────
# Precompiled headers
# ───────────────────────────────────────────────────────────────
set(PCH_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp")
set(PCH_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/pch.cpp")
file(WRITE ${PCH_SOURCE} "#include \"${PCH_HEADER}\"\n")

add_library(pch STATIC ${PCH_SOURCE})
target_precompile_headers(pch PRIVATE ${PCH_HEADER})

# ───────────────────────────────────────────────────────────────
# External libraries
# ───────────────────────────────────────────────────────────────

# CURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Boost
find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

# OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# libpq
include_directories(/usr/local/include)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)
include_directories(${LIBPQ_INCLUDE_DIRS})

# libpqxx
find_library(LIBPQXX_LIB NAMES pqxx PATHS /usr/local/lib)
if(NOT LIBPQXX_LIB)
  message(FATAL_ERROR "libpqxx not found")
endif()

# Hiredis
find_path(HIREDIS_HEADER hiredis)
find_library(HIREDIS_LIB hiredis)

# Redis-plus-plus
find_path(REDIS_PLUS_PLUS_HEADER sw)
find_library(REDIS_PLUS_PLUS_LIB redis++)

# ───────────────────────────────────────────────────────────────
# Compile API directory
# ───────────────────────────────────────────────────────────────
link_directories("lib")
include_directories("lib/include")

file(GLOB_RECURSE API_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/api/*.cpp")

foreach(SOURCE_FILE ${API_SOURCES})
  get_filename_component(LIB_NAME ${SOURCE_FILE} NAME_WE)
  add_library(
    ${LIB_NAME} SHARED ${SOURCE_FILE}
    server.cpp
    auth/session.cpp
    auth/httpclient.cpp
    auth/email.cpp
    db/redis.cpp
    db/postgres.cpp
    request/apikey.cpp
    request/request.cpp
    request/middleware.cpp
  )
  set_target_properties(
    ${LIB_NAME}
    PROPERTIES OUTPUT_NAME ${LIB_NAME}
    LIBRARY_OUTPUT_DIRECTORY "."
  )
  target_link_libraries(
    ${LIB_NAME}
    ${CURL_LIBRARIES}
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
    dl
    ${LIBPQXX_LIB}
    ${LIBPQ_LIBRARIES}
    ${HIREDIS_LIB}
    ${REDIS_PLUS_PLUS_LIB}
    pch
    bcrypt
  )
endforeach()

# ───────────────────────────────────────────────────────────────
# Executable
# ───────────────────────────────────────────────────────────────
add_executable(
  ReaderServer
  main.cpp
  server.cpp
  auth/email.cpp
  auth/httpclient.cpp
  db/redis.cpp
  db/postgres.cpp
)

target_link_libraries(
  ReaderServer
  ${CURL_LIBRARIES}
  ${Boost_LIBRARIES}
  OpenSSL::SSL
  OpenSSL::Crypto
  pthread
  dl
  ${LIBPQXX_LIB}
  ${LIBPQ_LIBRARIES}
  ${HIREDIS_LIB}
  ${REDIS_PLUS_PLUS_LIB}
  pch
)

set_target_properties(ReaderServer PROPERTIES LINK_FLAGS "-rdynamic")
set_target_properties(ReaderServer PROPERTIES ENABLE_EXPORTS ON)

foreach(SOURCE_FILE ${API_SOURCES})
  get_filename_component(LIB_NAME ${SOURCE_FILE} NAME_WE)
  target_link_libraries(${LIB_NAME} ReaderServer)
endforeach()
