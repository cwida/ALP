# ---------------------------------------------------------------------------
# BtrBlocks
# ---------------------------------------------------------------------------
project(Analyze_better_blocks)
cmake_minimum_required(VERSION 3.22)

# ---------------------------------------------------------------------------
# Environment
# ---------------------------------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
add_compile_options(-march=native)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-fstandalone-debug)
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (APPLE)
    list(APPEND CMAKE_PREFIX_PATH /usr/local/opt/bison)
    list(APPEND CMAKE_PREFIX_PATH /usr/local/opt/flex)
endif (APPLE)

if (CYGWIN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif (CYGWIN)

option(NO_SIMD "NO_SIMD" OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)

include("${CMAKE_SOURCE_DIR}/vendor/aws-sdk.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/benchmark.cmake")
#include("${CMAKE_SOURCE_DIR}/vendor/googletest.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/gflags.cmake")
#include("${CMAKE_SOURCE_DIR}/vendor/rapidjson.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/yaml-cpp.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/clang-tidy.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/fastpfor.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/spdlog.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/tbb.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/croaring.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/gdouble.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/turbo.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/lz4.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/fsst.cmake")

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------
include_directories(shared-headers)

include("${CMAKE_SOURCE_DIR}/shared-headers/local.cmake")
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${FLEX_INCLUDE_DIRS}
        ${GTEST_INCLUDE_DIR}
        ${GFLAGS_INCLUDE_DIR}
)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/playground/local.cmake")
add_subdirectory(cengine)
add_subdirectory(tools)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

# Tester does not link correctly because of tzt_stats
add_subdirectory(test)

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/benchmark/local.cmake")

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------

# add_executable(harbook harbook.cpp)
# add_dependencies(harbook cengine)
# target_link_libraries(harbook cengine psql)
# target_include_directories(harbook PRIVATE ${CENGINE_INCLUDE_DIR})
# ---------------------------------------------------------------------------
# Linting
# ---------------------------------------------------------------------------

add_custom_target(lint)
add_dependencies(lint ${lint_targets})

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

message(STATUS "[compiler] settings")
message(STATUS "     CMAKE_CXX_FLAGS            = ${CMAKE_CXX_FLAGS}")
message(STATUS "     CMAKE_CXX_FLAGS_RELEASE    = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "[cengine] settings")
message(STATUS "    GFLAGS_INCLUDE_DIR          = ${GFLAGS_INCLUDE_DIR}")
message(STATUS "    GFLAGS_LIBRARY_PATH         = ${GFLAGS_LIBRARY_PATH}")
message(STATUS "[TEST] settings")
message(STATUS "    GTEST_INCLUDE_DIR           = ${GTEST_INCLUDE_DIR}")
message(STATUS "    GTEST_LIBRARY_PATH          = ${GTEST_LIBRARY_PATH}")
message(STATUS "    CMAKE_BUILD_TYPE          = ${CMAKE_BUILD_TYPE}")
message(STATUS "    CMAKE_C_COMPILER          = ${CMAKE_C_COMPILER}")
message(STATUS "    CMAKE_CXX_COMPILER          = ${CMAKE_CXX_COMPILER}")
message(STATUS "    CMAKE_C_FLAGS_RELEASE    = ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "    CMAKE_CXX_STANDARD    = ${CMAKE_CXX_STANDARD}")
message(STATUS "    CMAKE_C_STANDARD    = ${CMAKE_C_STANDARD}")

# ---------------------------------------------------------------------------
# ALP
# ---------------------------------------------------------------------------
include("${CMAKE_SOURCE_DIR}/alp/manual/CMakeLists.txt")