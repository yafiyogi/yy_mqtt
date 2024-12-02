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

#include <cstddef>

#include <stdexcept>

#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_span.h"
#include "yy_cpp/yy_tokenizer.h"

#include "yy_mqtt_constants.h"
#include "yy_mqtt_types.h"
#include "yy_mqtt_util.h"

namespace yafiyogi::yy_mqtt {
namespace {

using tokenizer_type = yy_util::tokenizer<std::string_view::value_type>;
using token_type = tokenizer_type::token_type;

}

std::string_view topic_trim(const std::string_view p_topic) noexcept
{
  return yy_util::trim_right(yy_util::trim(p_topic), mqtt_detail::TopicLevelSeparator);
}

TopicLevelsView & topic_tokenize_view(TopicLevelsView & p_levels,
                                      const std::string_view p_topic) noexcept
{
  tokenizer_type tokenizer{yy_quad::make_const_span(p_topic),
                           mqtt_detail::TopicLevelSeparatorChar};
  p_levels.clear();
  p_levels.reserve(static_cast<std::size_t>(std::count(p_topic.begin(), p_topic.end(), mqtt_detail::TopicLevelSeparatorChar) + 1));

  while(!tokenizer.empty() || tokenizer.has_more())
  {
    auto level{tokenizer.scan()};
    p_levels.emplace_back(std::string_view{level.data(), level.size()});
  }

  return p_levels;
}

TopicLevelsView topic_tokenize_view(const std::string_view p_topic) noexcept
{
  TopicLevelsView levels;

  topic_tokenize_view(levels, p_topic);

  return levels;
}

TopicLevels & topic_tokenize(TopicLevels & p_levels,
                             const std::string_view p_topic) noexcept
{
  tokenizer_type tokenizer{yy_quad::make_const_span(p_topic),
                           mqtt_detail::TopicLevelSeparatorChar};
  p_levels.clear();
  p_levels.reserve(static_cast<std::size_t>(std::count(p_topic.begin(), p_topic.end(), mqtt_detail::TopicLevelSeparatorChar) + 1));

  while(!tokenizer.empty() || tokenizer.has_more())
  {
    auto level{tokenizer.scan()};
    p_levels.emplace_back(std::string{level.data(), level.size()});
  }

  return p_levels;
}

TopicLevels topic_tokenize(const std::string_view p_topic) noexcept
{
  TopicLevels levels;

  topic_tokenize(levels, p_topic);

  return levels;
}

TopicValidStatus topic_validate_level(std::string_view p_level,
                                      const TopicType p_type,
                                      const bool has_more)
{
  // mqtt-v5.0-os 4.7.1 Topic Wildcards
  // 2939: The wildcard characters can be used in Topic Filters, but MUST NOT be used within a Topic Name.
  if((std::string_view::npos != p_level.find(mqtt_detail::TopicSingleLevelWildcardChar))
     && ((TopicType::Name == p_type)
         || (mqtt_detail::TopicSingleLevelWildcard != p_level)))
  {
    return TopicValidStatus::Invalid;
  }

  // mqtt-v5.0-os 4.7.1 Topic Wildcards
  // 2939: The wildcard characters can be used in Topic Filters, but MUST NOT be used within a Topic Name.
  if((std::string_view::npos != p_level.find(mqtt_detail::TopicMultiLevelWildcardChar))
     && ((TopicType::Name == p_type)
         || (mqtt_detail::TopicMultiLevelWildcard != p_level)
         || has_more))
  {
    return TopicValidStatus::Invalid;
  }

  return TopicValidStatus::Valid;
}

TopicValidStatus topic_validate(std::string_view p_topic,
                                const TopicType p_type)
{
  if((p_type != TopicType::Name)
     && (p_type != TopicType::Filter))
  {
    return TopicValidStatus::BadParam;
  }

  tokenizer_type tokenizer{yy_quad::make_const_span(p_topic),
                           mqtt_detail::TopicLevelSeparatorChar};

  while(!tokenizer.empty() || tokenizer.has_more())
  {
    token_type level = tokenizer.scan();

    if(auto status = topic_validate_level(std::string_view{level.data(), level.size()},
                                          p_type,
                                          tokenizer.has_more());
       TopicValidStatus::Valid != status)
    {
      return status;
    }
  }

  return TopicValidStatus::Valid;
}

TopicValidStatus topic_validate(const TopicLevelsView & p_levels,
                                const TopicType p_type)
{
  if((p_type != TopicType::Name)
     && (p_type != TopicType::Filter))
  {
    return TopicValidStatus::BadParam;
  }

  const std::size_t max_levels = p_levels.size();
  std::size_t level_no = 0;


  for(const auto & level : p_levels)
  {
    ++level_no;

    if(auto status = topic_validate_level(level,
                                          p_type,
                                          max_levels != level_no);
       TopicValidStatus::Valid != status)
    {
      return status;
    }
  }

  return TopicValidStatus::Valid;
}

TopicMatchStatus topic_match(const std::string_view & p_filter,
                             const std::string_view & p_topic) noexcept
{
  auto filter{yy_quad::make_const_span(p_filter)};
  auto topic{yy_quad::make_const_span(p_topic)};

  tokenizer_type filter_tokenizer{filter,
                                  mqtt_detail::TopicLevelSeparatorChar};
  tokenizer_type topic_tokenizer{topic,
                                 mqtt_detail::TopicLevelSeparatorChar};

  if(!p_filter.empty()
     && !p_topic.empty())
  {
    auto filter_level_0{tokenizer_type::scan(filter, filter_tokenizer.delim())};
    auto topic_level_0{tokenizer_type::scan(topic, topic_tokenizer.delim())};

    if(((mqtt_detail::TopicMultiLevelWildcard == filter_level_0)
        || (mqtt_detail::TopicSingleLevelWildcard == filter_level_0))
       && !topic_level_0.empty()
       && (mqtt_detail::TopicSysChar == topic_level_0[0]))
    {
      return TopicMatchStatus::Fail;
    }
  }

  while(filter_tokenizer.has_more())
  {
    token_type filter_level{filter_tokenizer.scan()};

    if(mqtt_detail::TopicMultiLevelWildcard == filter_level)
    {
      return TopicMatchStatus::Match;
    }

    if(mqtt_detail::TopicSingleLevelWildcard == filter_level)
    {
      if(topic_tokenizer.empty())
      {
        return TopicMatchStatus::Match;
      }
      std::ignore = topic_tokenizer.scan();
    }
    else
    {
      if(topic_tokenizer.empty()
         || (filter_level != topic_tokenizer.scan()))
      {
        return TopicMatchStatus::Fail;
      }
    }
  }

  if(topic_tokenizer.has_more())
  {
    if(!p_filter.empty()
       && (mqtt_detail::TopicSingleLevelWildcard == filter_tokenizer.token()))
    {
      std::ignore = topic_tokenizer.scan();
      if(!topic_tokenizer.token().empty())
      {
        return TopicMatchStatus::Fail;
      }
    }
  }

  return TopicMatchStatus::Match;
}


TopicMatchStatus topic_match(const TopicLevelsView & p_filter,
                             const TopicLevelsView & p_topic) noexcept
{
  std::size_t filter_level_no = 0;
  const std::size_t max_topic_level = p_topic.size();
  std::size_t topic_level_no = 0;

  if(!p_filter.empty()
     && !p_topic.empty()
     && ((mqtt_detail::TopicMultiLevelWildcard == p_filter[0])
         || (mqtt_detail::TopicSingleLevelWildcard == p_filter[0]))
     && !p_topic[0].empty()
     && (mqtt_detail::TopicSysChar == p_topic[0][0]))
  {
    return TopicMatchStatus::Fail;
  }

  for(const auto & filter_level : p_filter)
  {
    if(mqtt_detail::TopicMultiLevelWildcard == filter_level)
    {
      return TopicMatchStatus::Match;
    }

    if(mqtt_detail::TopicSingleLevelWildcard == filter_level)
    {
      if(max_topic_level == topic_level_no)
      {
        return TopicMatchStatus::Match;
      }
    }
    else
    {
      if((max_topic_level == topic_level_no)
         || (filter_level != p_topic[topic_level_no]))
      {
        return TopicMatchStatus::Fail;
      }
    }

    ++filter_level_no;
    ++topic_level_no;
  }

  if(topic_level_no < max_topic_level)
  {
    if(!p_filter.empty()
       && (mqtt_detail::TopicSingleLevelWildcard == p_filter[filter_level_no - 1]))
    {
      if(!p_topic[topic_level_no].empty())
      {
        return TopicMatchStatus::Fail;
      }
    }
  }

  return TopicMatchStatus::Match;
}

} // namespace yafiyogi::yy_mqtt
