#
#
#  MIT License
#
#  Copyright (c) 2024 Yafiyogi
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#
#

cmake_minimum_required(VERSION 3.24)

project(benchmark_yy_mqtt LANGUAGES CXX)

include(${YY_CMAKE}/cmake_boost-1.84.0.txt)

find_package(Boost CONFIG 1.84.0 REQUIRED COMPONENTS locale)
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

target_compile_options(yy_mqtt
  PRIVATE
  "-DSPDLOG_COMPILED_LIB"
  "-DSPDLOG_FMT_EXTERNAL")

target_include_directories(benchmark_yy_mqtt
  PRIVATE
    "${PROJECT_SOURCE_DIR}/.."
    "${CMAKE_INSTALL_PREFIX}/include" )

target_link_directories(benchmark_yy_mqtt
  PRIVATE
  "${YY_THIRD_PARTY_LIBRARY}/lib"
  /opt/local/lib )

target_link_libraries(benchmark_yy_mqtt
  yy_mqtt
  yy_cpp::yy_cpp
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
