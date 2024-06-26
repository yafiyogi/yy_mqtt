/*

  MIT License

  Copyright (c) 2024 Yafiyogi

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

#include <stdexcept>

#include "fmt/format.h"

#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_tokenizer.h"

#include "yy_mqtt_constants.h"
#include "yy_mqtt_util.h"

namespace yafiyogi::yy_mqtt {

std::string_view topic_trim(const std::string_view p_topic) noexcept
{
  return yy_util::trim(yy_util::trim(p_topic), mqtt_detail::TopicLevelSeparator);
}

TopicLevels topic_tokenize(const std::string_view p_topic) noexcept
{
  yy_util::tokenizer<std::string_view::value_type> tokenizer{yy_quad::make_const_span(p_topic),
                                                             mqtt_detail::TopicLevelSeparatorChar};
  TopicLevels levels;

  while(!tokenizer.empty())
  {
    auto level = tokenizer.scan();
    levels.emplace_back(std::string_view{level.begin(), level.end()});
  }

  return levels;
}

bool topic_validate(const TopicLevels & p_levels,
                    const TopicType p_type)
{
  switch(p_type)
  {
    case TopicType::Name:
    case TopicType::Filter:
    {
      const size_t max_levels = p_levels.size();
      size_t level_no = 0;

      for(const auto & level : p_levels)
      {
        ++level_no;

        // mqtt-v5.0-os 4.7.1 Topic Wildcards
        // 2939: The wildcard characters can be used in Topic Filters, but MUST NOT be used within a Topic Name.
        if((std::string_view::npos != level.find(mqtt_detail::TopicSingleLevelWildcard))
           && ((TopicType::Name == p_type)
               || (mqtt_detail::TopicSingleLevelWildcard != level)))
        {
          return false;
        }

        // mqtt-v5.0-os 4.7.1 Topic Wildcards
        // 2939: The wildcard characters can be used in Topic Filters, but MUST NOT be used within a Topic Name.
        if((std::string_view::npos != level.find(mqtt_detail::TopicMultiLevelWildcard))
           && ((TopicType::Name == p_type)
               || (mqtt_detail::TopicMultiLevelWildcard != level)
               || (max_levels != level_no)))
        {
          return false;
        }
      }
      break;
    }

    default:
      throw std::runtime_error("MQTT topic_validate: topic type must be 'Name' or 'Filter'.");
  }

  return true;
}

bool topic_validate(const std::string_view p_topic,
                    const TopicType p_type)
{
  return topic_validate(topic_tokenize(p_topic), p_type);
}

bool topic_match(const TopicLevels & p_filter,
                 const TopicLevels & p_topic) noexcept
{
  size_t filter_level_no = 0;
  const size_t max_topic_level = p_topic.size();
  size_t topic_level_no = 0;

  if(!p_filter.empty()
     && !p_topic.empty()
     && ((mqtt_detail::TopicMultiLevelWildcard == p_filter[0])
         || (mqtt_detail::TopicSingleLevelWildcard == p_filter[0]))
     && !p_topic[0].empty()
     && (mqtt_detail::TopicSysChar == p_topic[0][0]))
  {
    return false;
  }

  for(const auto & filter_level : p_filter)
  {
    if(mqtt_detail::TopicMultiLevelWildcard == filter_level)
    {
      // Filter has '#' wildcard so success.
      return true;
    }

    if(max_topic_level == filter_level_no)
    {
      // There are more filter levels than topic levels, so fail.
      return false;
    }

    if((mqtt_detail::TopicSingleLevelWildcard != filter_level)
       && (p_topic[topic_level_no] != filter_level))
    {
      // Not filter level a '*' wildcard or filter level not topic level,
      // so fail.
      return false;
    }

    ++filter_level_no;
    ++topic_level_no;
  }

  // All topic levels matched?
  return filter_level_no == max_topic_level;
}

bool topic_match(const std::string_view & p_filter,
                 const std::string_view & p_topic) noexcept
{
  return topic_match(topic_tokenize(p_filter), topic_tokenize(p_topic));
}

} // namespace yafiyogi::yy_mqtt
