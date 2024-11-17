cmake_minimum_required(VERSION 3.24)

project(benchmark_yy_mqtt LANGUAGES CXX)

find_package(Boost 1.84.0 REQUIRED COMPONENTS locale)
find_package(fmt REQUIRED)
find_package(benchmark REQUIRED)
find_package(spdlog REQUIRED)
find_package(yy_cpp REQUIRED)

add_executable(benchmark_yy_mqtt
  bench_topics.cpp
  bench_flat_topics.cpp
  bench_fast_topics.cpp
  bench_faster_topics.cpp
  bench_state_topics.cpp
  bench_variant_state_topics.cpp

  bench_topic_tokenize.cpp
  bench_topic_tokenize_view.cpp

  bench_topic_validate.cpp

  bench_yy_mqtt.cpp )

target_include_directories(yy_mqtt
  PRIVATE

    "${YY_THIRD_PARTY_LIBRARY}" )
target_include_directories(benchmark_yy_mqtt
  PRIVATE
    "${CMAKE_INSTALL_PREFIX}/include"
    "${PROJECT_SOURCE_DIR}/.." )

target_compile_options(yy_mqtt
  PRIVATE
  "-DSPDLOG_COMPILED_LIB"
  "-DSPDLOG_FMT_EXTERNAL")

target_link_directories(benchmark_yy_mqtt
  PRIVATE
  "${YY_THIRD_PARTY_LIBRARY}/lib"
  /opt/local/lib )

target_link_libraries(benchmark_yy_mqtt
  yy_cpp::yy_cpp
  yy_mqtt
  benchmark::benchmark
  Boost::locale
  fmt::fmt
  spdlog::spdlog
  #profiler
  )

# Test target
add_custom_target(benchmark
  COMMAND
    ./benchmark_yy_mqtt)

add_dependencies(benchmark benchmark_yy_mqtt)
