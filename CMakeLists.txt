cmake_minimum_required(VERSION 3.24)

project(logger LANGUAGES CXX)

option(LOGGER_BUILD_EXAMPLES "Build logger examples" ON)
option(LOGGER_BUILD_TESTS "Build logger tests" ON)
option(LOGGER_BUILD_BENCHMARKS "Build logger benchmarks" OFF)

find_package(fmt REQUIRED)

add_library(logger
  src/formatter.cpp
  src/sink.cpp
  src/queue.cpp
  src/sync_backend.cpp
  src/async_backend.cpp
  src/logger.cpp
  src/parser.cpp
)

add_library(logger::logger ALIAS logger)

target_compile_features(logger PUBLIC cxx_std_20)

target_include_directories(logger
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(logger PUBLIC fmt::fmt)

if(LOGGER_BUILD_EXAMPLES)
  add_executable(logger_basic_example examples/basic.cpp)
  target_link_libraries(logger_basic_example PRIVATE logger::logger)
endif()

if(LOGGER_BUILD_TESTS)
  include(CTest)
  find_package(GTest QUIET)

  if(GTest_FOUND)
    add_executable(logger_tests tests/logger_tests.cpp)
    target_link_libraries(logger_tests PRIVATE logger::logger GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(logger_tests)
  endif()
endif()

if(LOGGER_BUILD_BENCHMARKS)
  find_package(benchmark QUIET)

  if(benchmark_FOUND)
    add_executable(logger_benchmark benchmark/logger_benchmark.cpp)
    target_link_libraries(logger_benchmark PRIVATE logger::logger benchmark::benchmark)
  endif()
endif()
