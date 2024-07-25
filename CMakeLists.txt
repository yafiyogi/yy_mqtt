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

# PKG_CONFIG_PATH=${HOME}/code/tp/lib/pkgconfig:${HOME}/code/tp/share/pkgconfig:${PKG_CONFIG_PATH} cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/code/cpp/build -DYY_THIRD_PARTY_LIBRARY=${HOME}/code/tp -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DCMAKE_BUILD_TYPE=Release .
# cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/code/cpp/build -DYY_THIRD_PARTY_LIBRARY=${HOME}/code/tp -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DCMAKE_BUILD_TYPE=Release .

cmake_minimum_required(VERSION 3.24)

project(yy_mqtt LANGUAGES CXX VERSION 0.0.1)

set(YY_ROOT "${PROJECT_SOURCE_DIR}/..")
set(YY_CMAKE "${YY_ROOT}/yy_cmake")

include(${YY_CMAKE}/cmake_common.txt)

include(${YY_CMAKE}/cmake_boost-1.84.0.txt)
include(${YY_CMAKE}/cmake_clang_tidy.txt)

find_package(Boost 1.84.0 REQUIRED COMPONENTS locale)

add_library(yy_mqtt STATIC)

target_include_directories(yy_mqtt
  PRIVATE
    "${CMAKE_INSTALL_PREFIX}/include"
    "${YY_THIRD_PARTY_LIBRARY}/include" )

target_sources(yy_mqtt
  PRIVATE
    yy_mqtt_util.cpp
  PUBLIC FILE_SET HEADERS
    FILES
      yy_mqtt_constants.h
      yy_mqtt_faster_topics.h
      yy_mqtt_types.h
      yy_mqtt_util.h)

add_subdirectory(unit_tests)
add_subdirectory(benchmarks)

install(TARGETS yy_mqtt
  EXPORT yy_mqttTargets
  FILE_SET HEADERS DESTINATION include/yy_mqtt)

install(EXPORT yy_mqttTargets
  FILE yy_mqttTargets.cmake
  NAMESPACE yy_mqtt::
  DESTINATION lib/cmake/yy_mqtt)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("yy_mqttConfigVersion.cmake"
  VERSION ${yy_mqtt_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES "yy_mqttConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/yy_mqttConfigVersion.cmake"
    DESTINATION lib/cmake/yy_mqtt)

get_target_property(YY_MQTT_HEADERS yy_mqtt HEADER_SET)
get_target_property(YY_MQTT_SOURCES yy_mqtt SOURCES)

add_custom_target(tidy-all
 COMMAND rm -f clang-tidy.log \; for f in "$$(" echo \"${YY_MQTT_HEADERS} ${YY_MQTT_SOURCES}\" | sed -n -e 's/;/ /pg' ")" \; do ${YY_CLANG_TIDY_CMD} ${YY_CLANG_TIDY_OPTIONS} "$$f" -- ${YY_TIDY_COMPILER_OPTIONS} >> clang-tidy.log \; done)
