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

#pragma once

#include <cstdint>

#include <string_view>

namespace yafiyogi::yy_mqtt {
namespace mqtt_detail {

inline constexpr std::string_view TopicLevelSeparator{"/"};
inline constexpr std::string_view::value_type TopicLevelSeparatorChar{TopicLevelSeparator[0]};
inline constexpr std::string_view TopicSingleLevelWildcard{"+"};
inline constexpr std::string_view::value_type TopicSingleLevelWildcardChar{TopicSingleLevelWildcard[0]};
inline constexpr std::string_view TopicMultiLevelWildcard{"#"};
inline constexpr std::string_view::value_type TopicMultiLevelWildcardChar{TopicMultiLevelWildcard[0]};
inline constexpr std::string_view TopicSys{"$"};
inline constexpr std::string_view::value_type TopicSysChar{TopicSys[0]};

} // namespace yafiyogi::yy_mqtt

enum class TopicType:uint8_t {
  Name,
  Filter
};

inline constexpr int mqtt_default_port = 1883;

} // namespace yafiyogi::yy_mqtt
