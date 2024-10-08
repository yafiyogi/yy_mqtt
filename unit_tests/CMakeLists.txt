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

project(test_yy_mqtt LANGUAGES CXX)

find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(yy_cpp REQUIRED)
find_package(yy_test REQUIRED)

add_executable(test_yy_mqtt
  fast_topic_tests.cpp
  faster_topic_tests.cpp
  state_topic_tests.cpp
  variant_state_topic_tests.cpp
  flat_topic_tests.cpp
  topic_tests.cpp
  topic_util_tests.cpp )

target_include_directories(test_yy_mqtt
  PRIVATE
    "${PROJECT_SOURCE_DIR}/.."
    "${CMAKE_INSTALL_PREFIX}/include"
    "${YY_THIRD_PARTY_LIBRARY}/include" )

target_link_directories(test_yy_mqtt
  PUBLIC
  /opt/local/lib )

target_compile_options(yy_mqtt
  PRIVATE
  "-DSPDLOG_COMPILED_LIB"
  "-DSPDLOG_FMT_EXTERNAL")

target_link_libraries(test_yy_mqtt
  yy_mqtt
  yy_test::yy_test
  yy_cpp::yy_cpp
  Boost::locale
  fmt::fmt
  spdlog::spdlog
  GTest::gtest
  GTest::gtest_main)

# Test target
add_custom_target(test
  COMMAND
    ./test_yy_mqtt)

add_dependencies(test test_yy_mqtt)

# Coverage target
cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH YY_PROJECT_SOURCE_PARENT_DIR)
add_custom_target(coverage
  COMMAND ${FASTCOV_CMD} --include "${YY_PROJECT_SOURCE_PARENT_DIR}" --exclude "${PROJECT_SOURCE_DIR}" --search-directory .. --process-gcno --output mqtt-gcov.json
  COMMAND ${FASTCOV_CMD} -C mqtt-gcov.json --lcov --output mqtt-gcov.info
  COMMAND genhtml -o mqtt-gcov.html mqtt-gcov.info )

add_dependencies(coverage test)

add_custom_target(asan
  COMMAND ./test_yy_mqtt --gtest_brief=1 heap_leak 2> test_yy_mqtt.asan_heap_leak.log
  COMMAND ./test_yy_mqtt --gtest_brief=1 global_buffer_overflow 2> test_yy_mqtt.asan_global_buffer_overflow.log)

add_dependencies(asan test_yy_mqtt)

add_custom_target(lsan
  COMMAND ./test_yy_mqtt --gtest_brief=1 heap_leak 2> test_yy_mqtt.lsan_heap_leak.log
  COMMAND ./test_yy_mqtt --gtest_brief=1 heap_buffer_overflow 2> test_yy_mqtt.lsan_heap_buffer_overflow.log )

add_dependencies(lsan test_yy_mqtt)
