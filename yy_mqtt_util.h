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

#include <string_view>

#include "yy_mqtt_constants.h"
#include "yy_mqtt_types.h"

namespace yafiyogi::yy_mqtt {

std::string_view topic_trim(const std::string_view p_topic) noexcept;
TopicLevelsView & topic_tokenize_view(TopicLevelsView & p_levels,
                                      const std::string_view p_topic) noexcept;
TopicLevelsView topic_tokenize_view(const std::string_view p_topic) noexcept;
TopicLevels & topic_tokenize(TopicLevels & p_levels,
                             const std::string_view p_topic) noexcept;
TopicLevels topic_tokenize(const std::string_view p_topic) noexcept;
enum class TopicValidStatus { Valid, Invalid, BadParam};
TopicValidStatus topic_validate(std::string_view topic,
                        const TopicType p_type);
TopicValidStatus topic_validate(const TopicLevelsView & p_levels,
                           const TopicType p_type);
enum class TopicMatchStatus { Match, Fail, Continue};
TopicMatchStatus topic_match(const std::string_view & p_filter,
                                 const std::string_view & p_topic) noexcept;
TopicMatchStatus topic_match(const TopicLevelsView & p_filter,
                             const TopicLevelsView & p_topic) noexcept;

} // namespace yafiyogi::yy_mqtt
