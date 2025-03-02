/*

  MIT License

  Copyright (c) 2024-2025 Yafiyogi

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <string_view>
#include <array>
#include <algorithm>
#include <memory>
#include <random>
#include <vector>

#include "fmt/format.h"

#include "yy_cpp/yy_assert.h"
#include "yy_cpp/yy_tokenizer.h"

#include "bench_yy_mqtt.h"

namespace yafiyogi::benchmark {
namespace {

template<typename T,
         typename ArrayT,
         size_type Idx>
std::array<T, Idx> shuffle_array(const ArrayT (& arr_in)[Idx]) noexcept
{
  std::array<T, Idx> arr{};
  for(size_type idx = 0; idx < Idx; ++idx)
  {
    arr[idx] = arr_in[idx];
  }

  std::random_device rd;
  std::mt19937 g(rd());

  std::shuffle(arr.begin(), arr.end(), g);

  return arr;
}

auto g_query = shuffle_array<std::string_view>({
    "iot21/Attic/TRV/availability",
    "iot21/Attic/TRV/battery",
    "iot21/Attic/TRV/boost_heating",
    "iot21/Attic/TRV/boost_heating_countdown",
    "iot21/Attic/TRV/boost_heating_countdown_time_set",
    "iot21/Attic/TRV/child_lock",
    "iot21/Attic/TRV/current_heating_setpoint",
    "iot21/Attic/TRV/eco_mode",
    "iot21/Attic/TRV/eco_temperature",
    "iot21/Attic/TRV/linkquality",
    "iot21/Attic/TRV/local_temperature",
    "iot21/Attic/TRV/local_temperature_calibration",
    "iot21/Attic/TRV/max_temperature",
    "iot21/Attic/TRV/min_temperature",
    "iot21/Attic/TRV/position",
    "iot21/Attic/TRV/preset",
    "iot21/Attic/TRV/programming_mode",
    "iot21/Attic/TRV/running_state",
    "iot21/Attic/TRV/set",
    "iot21/Attic/TRV/system_mode",
    "iot21/Attic/TRV/update-installed_version",
    "iot21/Attic/TRV/update-latest_version",
    "iot21/Attic/TRV/update-state",
    "iot21/Attic/TRV/valve_state",
    "iot21/Attic/TRV/window",
    "iot21/Attic/TRV/window_detection",
    "iot21/Attic/TRV",
    "iot21/Attic/Temp/availability",
    "iot21/Attic/Temp/battery",
    "iot21/Attic/Temp/humidity",
    "iot21/Attic/Temp/linkquality",
    "iot21/Attic/Temp/temperature",
    "iot21/Attic/Temp/voltage",
    "iot21/Attic/Temp",
    "iot21/Back/Temp/availability",
    "iot21/Back/Temp/battery",
    "iot21/Back/Temp/humidity",
    "iot21/Back/Temp/linkquality",
    "iot21/Back/Temp/temperature",
    "iot21/Back/Temp/voltage",
    "iot21/Back/Temp",
    "iot21/Bathroom/Temp/availability",
    "iot21/Bathroom/Temp/battery",
    "iot21/Bathroom/Temp/humidity",
    "iot21/Bathroom/Temp/linkquality",
    "iot21/Bathroom/Temp/temperature",
    "iot21/Bathroom/Temp/voltage",
    "iot21/Bathroom/Temp",
    "iot21/Christmas Tree Lights/availability",
    "iot21/Christmas Tree Lights/current",
    "iot21/Christmas Tree Lights/energy",
    "iot21/Christmas Tree Lights/linkquality",
    "iot21/Christmas Tree Lights/power",
    "iot21/Christmas Tree Lights/power_on_behavior",
    "iot21/Christmas Tree Lights/state",
    "iot21/Christmas Tree Lights/voltage",
    "iot21/Christmas Tree Lights",
    "iot21/Dining Room/Temp/availability",
    "iot21/Dining Room/Temp/battery",
    "iot21/Dining Room/Temp/humidity",
    "iot21/Dining Room/Temp/linkquality",
    "iot21/Dining Room/Temp/temperature",
    "iot21/Dining Room/Temp/voltage",
    "iot21/Dining Room/Temp",
    "iot21/Front Bedroom/Light/availability",
    "iot21/Front Bedroom/Light/brightness",
    "iot21/Front Bedroom/Light/level_config-on_level",
    "iot21/Front Bedroom/Light/linkquality",
    "iot21/Front Bedroom/Light/power_on_behavior",
    "iot21/Front Bedroom/Light/state",
    "iot21/Front Bedroom/Light/update-installed_version",
    "iot21/Front Bedroom/Light/update-latest_version",
    "iot21/Front Bedroom/Light/update-progress",
    "iot21/Front Bedroom/Light/update-remaining",
    "iot21/Front Bedroom/Light/update-state",
    "iot21/Front Bedroom/Light",
    "iot21/Front Bedroom/Plug/Desk Fan/linkquality",
    "iot21/Front Bedroom/Plug/Desk Fan/state",
    "iot21/Front Bedroom/Plug/Desk Fan/update-installed_version",
    "iot21/Front Bedroom/Plug/Desk Fan/update-latest_version",
    "iot21/Front Bedroom/Plug/Desk Fan/update-state",
    "iot21/Front Bedroom/Plug/Desk Fan",
    "iot21/Front Bedroom/Plug/Heat Pad/availability",
    "iot21/Front Bedroom/Plug/Heat Pad/linkquality",
    "iot21/Front Bedroom/Plug/Heat Pad/power_on_behavior",
    "iot21/Front Bedroom/Plug/Heat Pad/state",
    "iot21/Front Bedroom/Plug/Heat Pad/update-installed_version",
    "iot21/Front Bedroom/Plug/Heat Pad/update-latest_version",
    "iot21/Front Bedroom/Plug/Heat Pad/update-state",
    "iot21/Front Bedroom/Plug/Heat Pad",
    "iot21/Front Bedroom/Plug/Salt Lamp/availability",
    "iot21/Front Bedroom/Plug/Salt Lamp/energy",
    "iot21/Front Bedroom/Plug/Salt Lamp/linkquality",
    "iot21/Front Bedroom/Plug/Salt Lamp/power",
    "iot21/Front Bedroom/Plug/Salt Lamp/power_on_behavior",
    "iot21/Front Bedroom/Plug/Salt Lamp/state",
    "iot21/Front Bedroom/Plug/Salt Lamp",
    "iot21/Front Bedroom/Switch/action_rate",
    "iot21/Front Bedroom/Switch/availability",
    "iot21/Front Bedroom/Switch/battery",
    "iot21/Front Bedroom/Switch/linkquality",
    "iot21/Front Bedroom/Switch/update-installed_version",
    "iot21/Front Bedroom/Switch/update-latest_version",
    "iot21/Front Bedroom/Switch/update-progress",
    "iot21/Front Bedroom/Switch/update-remaining",
    "iot21/Front Bedroom/Switch/update-state",
    "iot21/Front Bedroom/Switch",
    "iot21/Front Bedroom/TRV/availability",
    "iot21/Front Bedroom/TRV/battery",
    "iot21/Front Bedroom/TRV/boost_heating",
    "iot21/Front Bedroom/TRV/boost_heating_countdown",
    "iot21/Front Bedroom/TRV/boost_heating_countdown_time_set",
    "iot21/Front Bedroom/TRV/child_lock",
    "iot21/Front Bedroom/TRV/current_heating_setpoint",
    "iot21/Front Bedroom/TRV/eco_mode",
    "iot21/Front Bedroom/TRV/eco_temperature",
    "iot21/Front Bedroom/TRV/linkquality",
    "iot21/Front Bedroom/TRV/local_temperature",
    "iot21/Front Bedroom/TRV/local_temperature_calibration",
    "iot21/Front Bedroom/TRV/max_temperature",
    "iot21/Front Bedroom/TRV/min_temperature",
    "iot21/Front Bedroom/TRV/position",
    "iot21/Front Bedroom/TRV/preset",
    "iot21/Front Bedroom/TRV/programming_mode",
    "iot21/Front Bedroom/TRV/running_state",
    "iot21/Front Bedroom/TRV/system_mode",
    "iot21/Front Bedroom/TRV/update-installed_version",
    "iot21/Front Bedroom/TRV/update-latest_version",
    "iot21/Front Bedroom/TRV/update-state",
    "iot21/Front Bedroom/TRV/valve_state",
    "iot21/Front Bedroom/TRV/window",
    "iot21/Front Bedroom/TRV/window_detection",
    "iot21/Front Bedroom/TRV",
    "iot21/Front Bedroom/Temp/availability",
    "iot21/Front Bedroom/Temp/battery",
    "iot21/Front Bedroom/Temp/humidity",
    "iot21/Front Bedroom/Temp/linkquality",
    "iot21/Front Bedroom/Temp/temperature",
    "iot21/Front Bedroom/Temp/voltage",
    "iot21/Front Bedroom/Temp",
    "iot21/Front/Motion/availability",
    "iot21/Front/Motion/battery",
    "iot21/Front/Motion/battery_low",
    "iot21/Front/Motion/linkquality",
    "iot21/Front/Motion/occupancy",
    "iot21/Front/Motion/tamper",
    "iot21/Front/Motion/voltage",
    "iot21/Front/Motion",
    "iot21/Front/Plug/Fairy Lights/availability",
    "iot21/Front/Plug/Fairy Lights/linkquality",
    "iot21/Front/Plug/Fairy Lights/power_on_behavior",
    "iot21/Front/Plug/Fairy Lights/state",
    "iot21/Front/Plug/Fairy Lights/update-installed_version",
    "iot21/Front/Plug/Fairy Lights/update-latest_version",
    "iot21/Front/Plug/Fairy Lights/update-progress",
    "iot21/Front/Plug/Fairy Lights/update-remaining",
    "iot21/Front/Plug/Fairy Lights/update-state",
    "iot21/Front/Plug/Fairy Lights",
    "iot21/Front/Plug/Floor Lamp/linkquality",
    "iot21/Front/Plug/Floor Lamp/state",
    "iot21/Front/Plug/Floor Lamp/update-installed_version",
    "iot21/Front/Plug/Floor Lamp/update-latest_version",
    "iot21/Front/Plug/Floor Lamp/update-progress",
    "iot21/Front/Plug/Floor Lamp/update-remaining",
    "iot21/Front/Plug/Floor Lamp/update-state",
    "iot21/Front/Plug/Floor Lamp",
    "iot21/Front/Plug/Heat Pad/availability",
    "iot21/Front/Plug/Heat Pad/current",
    "iot21/Front/Plug/Heat Pad/energy",
    "iot21/Front/Plug/Heat Pad/linkquality",
    "iot21/Front/Plug/Heat Pad/power",
    "iot21/Front/Plug/Heat Pad/power_on_behavior",
    "iot21/Front/Plug/Heat Pad/state",
    "iot21/Front/Plug/Heat Pad/update-installed_version",
    "iot21/Front/Plug/Heat Pad/update-latest_version",
    "iot21/Front/Plug/Heat Pad/update-state",
    "iot21/Front/Plug/Heat Pad/voltage",
    "iot21/Front/Plug/Heat Pad",
    "iot21/Front/Plug/TV Lamp/availability",
    "iot21/Front/Plug/TV Lamp/linkquality",
    "iot21/Front/Plug/TV Lamp/power_on_behavior",
    "iot21/Front/Plug/TV Lamp/state",
    "iot21/Front/Plug/TV Lamp/update-installed_version",
    "iot21/Front/Plug/TV Lamp/update-latest_version",
    "iot21/Front/Plug/TV Lamp/update-progress",
    "iot21/Front/Plug/TV Lamp/update-remaining",
    "iot21/Front/Plug/TV Lamp/update-state",
    "iot21/Front/Plug/TV Lamp",
    "iot21/Front/Temp/availability",
    "iot21/Front/Temp/battery",
    "iot21/Front/Temp/humidity",
    "iot21/Front/Temp/linkquality",
    "iot21/Front/Temp/power_outage_count",
    "iot21/Front/Temp/pressure",
    "iot21/Front/Temp/temperature",
    "iot21/Front/Temp/voltage",
    "iot21/Front/Temp",
    "iot21/H's Bedroom/TRV/availability",
    "iot21/H's Bedroom/TRV/battery",
    "iot21/H's Bedroom/TRV/boost_heating",
    "iot21/H's Bedroom/TRV/boost_heating_countdown",
    "iot21/H's Bedroom/TRV/boost_heating_countdown_time_set",
    "iot21/H's Bedroom/TRV/child_lock",
    "iot21/H's Bedroom/TRV/current_heating_setpoint",
    "iot21/H's Bedroom/TRV/eco_mode",
    "iot21/H's Bedroom/TRV/eco_temperature",
    "iot21/H's Bedroom/TRV/linkquality",
    "iot21/H's Bedroom/TRV/local_temperature",
    "iot21/H's Bedroom/TRV/local_temperature_calibration",
    "iot21/H's Bedroom/TRV/max_temperature",
    "iot21/H's Bedroom/TRV/min_temperature",
    "iot21/H's Bedroom/TRV/position",
    "iot21/H's Bedroom/TRV/preset",
    "iot21/H's Bedroom/TRV/programming_mode",
    "iot21/H's Bedroom/TRV/running_state",
    "iot21/H's Bedroom/TRV/system_mode",
    "iot21/H's Bedroom/TRV/update-installed_version",
    "iot21/H's Bedroom/TRV/update-latest_version",
    "iot21/H's Bedroom/TRV/update-state",
    "iot21/H's Bedroom/TRV/valve_state",
    "iot21/H's Bedroom/TRV/window",
    "iot21/H's Bedroom/TRV/window_detection",
    "iot21/H's Bedroom/TRV",
    "iot21/H's Bedroom/Temp/availability",
    "iot21/H's Bedroom/Temp/battery",
    "iot21/H's Bedroom/Temp/humidity",
    "iot21/H's Bedroom/Temp/linkquality",
    "iot21/H's Bedroom/Temp/temperature",
    "iot21/H's Bedroom/Temp/voltage",
    "iot21/H's Bedroom/Temp",
    "iot21/Hall/Plug/Desk Light/linkquality",
    "iot21/Hall/Plug/Desk Light/state",
    "iot21/Hall/Plug/Desk Light/update-installed_version",
    "iot21/Hall/Plug/Desk Light/update-latest_version",
    "iot21/Hall/Plug/Desk Light/update-state",
    "iot21/Hall/Plug/Desk Light",
    "iot21/Hall/Plug/Lamp/availability",
    "iot21/Hall/Plug/Lamp/linkquality",
    "iot21/Hall/Plug/Lamp/power_on_behavior",
    "iot21/Hall/Plug/Lamp/state",
    "iot21/Hall/Plug/Lamp/update-installed_version",
    "iot21/Hall/Plug/Lamp/update-latest_version",
    "iot21/Hall/Plug/Lamp/update-state",
    "iot21/Hall/Plug/Lamp",
    "iot21/Hall/Switch/Hall Lamp/battery",
    "iot21/Hall/Switch/Hall Lamp/linkquality",
    "iot21/Hall/Switch/Hall Lamp/update-installed_version",
    "iot21/Hall/Switch/Hall Lamp/update-latest_version",
    "iot21/Hall/Switch/Hall Lamp/update-state",
    "iot21/Hall/Switch/Hall Lamp",
    "iot21/Hall/Switch/Lamp/action_rate",
    "iot21/Hall/Switch/Lamp/availability",
    "iot21/Hall/Switch/Lamp/battery",
    "iot21/Hall/Switch/Lamp/linkquality",
    "iot21/Hall/Switch/Lamp/update-installed_version",
    "iot21/Hall/Switch/Lamp/update-latest_version",
    "iot21/Hall/Switch/Lamp/update-progress",
    "iot21/Hall/Switch/Lamp/update-remaining",
    "iot21/Hall/Switch/Lamp/update-state",
    "iot21/Hall/Switch/Lamp",
    "iot21/Hall/Temp/availability",
    "iot21/Hall/Temp/battery",
    "iot21/Hall/Temp/humidity",
    "iot21/Hall/Temp/linkquality",
    "iot21/Hall/Temp/temperature",
    "iot21/Hall/Temp/voltage",
    "iot21/Hall/Temp",
    "iot21/Kitchen/Temp/availability",
    "iot21/Kitchen/Temp/battery",
    "iot21/Kitchen/Temp/humidity",
    "iot21/Kitchen/Temp/linkquality",
    "iot21/Kitchen/Temp/temperature",
    "iot21/Kitchen/Temp/voltage",
    "iot21/Kitchen/Temp",
    "iot21/Study/AirQM/availability",
    "iot21/Study/AirQM/humidity",
    "iot21/Study/AirQM/linkquality",
    "iot21/Study/AirQM/pm25",
    "iot21/Study/AirQM/temperature",
    "iot21/Study/AirQM/update-installed_version",
    "iot21/Study/AirQM/update-latest_version",
    "iot21/Study/AirQM/update-progress",
    "iot21/Study/AirQM/update-remaining",
    "iot21/Study/AirQM/update-state",
    "iot21/Study/AirQM/voc_index",
    "iot21/Study/AirQM",
    "iot21/Study/Motion/availability",
    "iot21/Study/Motion/battery",
    "iot21/Study/Motion/battery_low",
    "iot21/Study/Motion/linkquality",
    "iot21/Study/Motion/occupancy",
    "iot21/Study/Motion/tamper",
    "iot21/Study/Motion/voltage",
    "iot21/Study/Motion",
    "iot21/Study/TRV/availability",
    "iot21/Study/TRV/battery",
    "iot21/Study/TRV/boost_heating",
    "iot21/Study/TRV/boost_heating_countdown",
    "iot21/Study/TRV/boost_heating_countdown_time_set",
    "iot21/Study/TRV/child_lock",
    "iot21/Study/TRV/current_heating_setpoint",
    "iot21/Study/TRV/eco_mode",
    "iot21/Study/TRV/eco_temperature",
    "iot21/Study/TRV/linkquality",
    "iot21/Study/TRV/local_temperature",
    "iot21/Study/TRV/local_temperature_calibration",
    "iot21/Study/TRV/max_temperature",
    "iot21/Study/TRV/min_temperature",
    "iot21/Study/TRV/position",
    "iot21/Study/TRV/preset",
    "iot21/Study/TRV/programming_mode",
    "iot21/Study/TRV/running_state",
    "iot21/Study/TRV/system_mode",
    "iot21/Study/TRV/update-installed_version",
    "iot21/Study/TRV/update-latest_version",
    "iot21/Study/TRV/update-state",
    "iot21/Study/TRV/valve_state",
    "iot21/Study/TRV/window",
    "iot21/Study/TRV/window_detection",
    "iot21/Study/TRV",
    "iot21/Study/Temp/availability",
    "iot21/Study/Temp/battery",
    "iot21/Study/Temp/humidity",
    "iot21/Study/Temp/linkquality",
    "iot21/Study/Temp/temperature",
    "iot21/Study/Temp/voltage",
    "iot21/Study/Temp",
    "iot21/Toilet/TRV/availability",
    "iot21/Toilet/TRV/battery",
    "iot21/Toilet/TRV/boost_heating",
    "iot21/Toilet/TRV/boost_heating_countdown",
    "iot21/Toilet/TRV/boost_heating_countdown_time_set",
    "iot21/Toilet/TRV/child_lock",
    "iot21/Toilet/TRV/current_heating_setpoint",
    "iot21/Toilet/TRV/eco_mode",
    "iot21/Toilet/TRV/eco_temperature",
    "iot21/Toilet/TRV/linkquality",
    "iot21/Toilet/TRV/local_temperature",
    "iot21/Toilet/TRV/local_temperature_calibration",
    "iot21/Toilet/TRV/max_temperature",
    "iot21/Toilet/TRV/min_temperature",
    "iot21/Toilet/TRV/position",
    "iot21/Toilet/TRV/preset",
    "iot21/Toilet/TRV/programming_mode",
    "iot21/Toilet/TRV/running_state",
    "iot21/Toilet/TRV/system_mode",
    "iot21/Toilet/TRV/update-installed_version",
    "iot21/Toilet/TRV/update-latest_version",
    "iot21/Toilet/TRV/update-state",
    "iot21/Toilet/TRV/valve_state",
    "iot21/Toilet/TRV/window",
    "iot21/Toilet/TRV/window_detection",
    "iot21/Toilet/TRV",
    "iot21/Toilet/Temp/availability",
    "iot21/Toilet/Temp/battery",
    "iot21/Toilet/Temp/humidity",
    "iot21/Toilet/Temp/linkquality",
    "iot21/Toilet/Temp/temperature",
    "iot21/Toilet/Temp/voltage",
    "iot21/Toilet/Temp",
    "iot21/Utility Room/Temp/availability",
    "iot21/Utility Room/Temp/battery",
    "iot21/Utility Room/Temp/humidity",
    "iot21/Utility Room/Temp/linkquality",
    "iot21/Utility Room/Temp/temperature",
    "iot21/Utility Room/Temp/voltage",
    "iot21/Utility Room/Temp",
    "iot21/bridge/devices",
    "iot21/bridge/extensions",
    "iot21/bridge/groups",
    "iot21/bridge/info",
    "iot21/bridge/state",
  });

auto topics = shuffle_array<std::string_view>({
    "iot21/#",
    "iot21/+/Temp",
    "iot21/Attic/TRV",
    "iot21/Attic/Temp",
    "iot21/Back/Temp",
    "iot21/Bathroom/Temp",
    "iot21/Christmas Tree Lights",
    "iot21/Dining Room/Temp",
    "iot21/Front Bedroom/#",
    "iot21/Front Bedroom/Light",
    "iot21/Front Bedroom/Plug/Desk Fan"
    "iot21/Front Bedroom/Plug/Heat Pad"
    "iot21/Front Bedroom/Plug/Salt Lamp",
    "iot21/Front Bedroom/Switch",
    "iot21/Front Bedroom/TRV",
    "iot21/Front Bedroom/Temp",
    "iot21/Front/Motion",
    "iot21/Front/Plug/Fairy Lights",
    "iot21/Front/Plug/Floor Lamp",
    "iot21/Front/Plug/Heat Pad",
    "iot21/Front/Plug/TV Lamp",
    "iot21/Front/Temp",
    "iot21/H's Bedroom/TRV",
    "iot21/H's Bedroom/Temp",
    "iot21/Hall/Plug/Desk Light",
    "iot21/Hall/Plug/Lamp",
    "iot21/Hall/Switch/Hall Lamp",
    "iot21/Hall/Switch/Lamp",
    "iot21/Hall/Temp",
    "iot21/Kitchen/Temp",
    "iot21/Study/AirQM",
    "iot21/Study/Motion",
    "iot21/Study/TRV",
    "iot21/Study/Temp",
    "iot21/Toilet/TRV",
    "iot21/Toilet/Temp",
    "iot21/Utility Room/Temp",
    });

} // anonymous namespace

Topics TopicsFixtureType::m_topics;
FlatTopics TopicsFixtureType::m_flat_topics;
FastTopics TopicsFixtureType::m_fast_topics;
FasterTopics TopicsFixtureType::m_faster_topics;
StateTopics TopicsFixtureType::m_state_topics;
VariantStateTopics TopicsFixtureType::m_variant_state_topics;

TopicsFixtureType::TopicsFixtureType()
{
}

void TopicsFixtureType::SetUp(const ::benchmark::State & /* st */)
{
  static bool done = false;

  if(!done)
  {
    done = true;
    int count = 0;
    for(auto topic: topics)
    {
      ++count;

      m_topics.add(topic, count);
      m_flat_topics.add(topic, count);
      m_fast_topics.add(topic, count);

      m_faster_topics.add(topic, count);
      m_state_topics.add(topic, count);
      m_variant_state_topics.add(topic, count);
    }
  }
}

std::string_view TopicsFixtureType::query(size_type idx)
{
  return g_query[idx];
}

size_type TopicsFixtureType::query_size()
{
  return g_query.size();
}

size_type TopicsFixtureType::topics_size()
{
  return topics.size();
}

} // namespace yafiyogi::benchmark

BENCHMARK_MAIN();
