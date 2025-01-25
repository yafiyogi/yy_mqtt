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

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "yy_cpp/yy_trie.h"

#include "yy_mqtt_constants.h"

namespace yafiyogi::yy_mqtt {
namespace topics_detail {

template<typename ValueType>
struct topics_traits final
{
    using trie_node_traits = yy_data::trie_detail::trie_node_traits<char, ValueType>;
    using label_type = typename trie_node_traits::label_type;
    using node_type = typename trie_node_traits::node_type;
    using node_ptr = typename trie_node_traits::node_ptr;
    using root_node_ptr = typename trie_node_traits::root_node_ptr;
    using node_edge = typename trie_node_traits::node_edge;
    using value_type = typename trie_node_traits::value_type;
};

template<typename ValueType>
class Query final
{
  public:
    using traits = topics_traits<ValueType>;
    using label_type = typename traits::label_type;
    using node_type = typename traits::node_type;
    using node_ptr = typename traits::node_ptr;
    using root_node_ptr = typename traits::root_node_ptr;
    using node_edge = typename traits::node_edge;
    using queue = std::vector<std::tuple<label_type, const node_type *>>;
    using value_type = typename traits::value_type;
    using payloads_type = std::vector<const node_type *>;

    constexpr explicit Query(root_node_ptr p_root) noexcept:
      m_root(std::move(p_root))
    {
    }

    Query() = delete;
    Query(const Query &) = delete;
    Query(Query &&) = delete;
    ~Query() = default;

    Query & operator=(const Query &) = delete;
    Query & operator=(Query &&) = delete;

    template<typename Visitor>
    constexpr bool find(std::string_view topic,
                        Visitor && visitor) noexcept
    {
      bool found = false;
      m_search_states.clear();
      add_state(label_type{}, m_root.get(), m_search_states);

      if(!topic.empty())
      {
        if(detail::TopicSysChar != topic[0])
        {
          add_wildcards(m_root.get(), m_search_states);
        }

        payloads_type payloads;
        payloads.reserve(3);

        const auto max = topic.size();
        for(size_t idx = 0; idx < max; ++idx)
        {
          bool more = next(topic[idx], idx == (max - 1), payloads);

          if(!payloads.empty())
          {
            found = true;
            for(const auto & payload: payloads)
            {
              visitor(payload->value());
            }
          }

          payloads.clear();

          if(!more)
          {
            break;
          }
        }
      }

      return found;
    }

  private:
    static constexpr bool add_wildcards(const node_type * p_node,
                                        queue & p_states_list) noexcept
    {
      bool added = false;

      if(add_state(detail::TopicSingleLevelWildcardChar, p_node->get(detail::TopicSingleLevelWildcardChar), p_states_list))
      {
        added = true;
      }

      if(add_state(detail::TopicMultiLevelWildcardChar, p_node->get(detail::TopicMultiLevelWildcardChar), p_states_list))
      {
        added = true;
      }

      return added;
    }

    static constexpr void add_payload(const node_type * p_node,
                                      payloads_type & p_payloads) noexcept
    {
      if(!p_node->empty())
      {
        p_payloads.emplace_back(p_node);
      }
    }

    static constexpr bool add_state(const label_type label,
                                    const node_type * p_state,
                                    queue & p_states_list) noexcept
    {
      if(p_state)
      {
        p_states_list.emplace_back(label, p_state);
      }

      return nullptr != p_state;
    }

    constexpr bool next(const char p_ch,
                        const bool p_last,
                        payloads_type & payloads) noexcept
    {
      queue new_states;

      while(!m_search_states.empty())
      {
        auto [label, state] = m_search_states.front();
        m_search_states.erase(m_search_states.begin());

        switch(label)
        {
          case detail::TopicSingleLevelWildcardChar:
            // Match using +
            // Re-add current node unless current ch is a level separator

            if(detail::TopicLevelSeparatorChar == p_ch)
            {
              // Finished matching '+'
              if(const auto separator = state->get(detail::TopicLevelSeparatorChar);
                 separator)
              {
                if(p_last)
                {
                  // Match xxx using xxx/+ or xxx/#
                  add_wildcards(separator, m_search_states);
                }
                else
                {
                  // Finished matching +, match / next.
                  add_state(detail::TopicLevelSeparatorChar, separator, new_states);
                }
              }
              else if(p_last)
              {
                // Finished matching +.
                add_payload(state, payloads);
              }
            }
            else if(p_last)
            {
              // Finished matching +.
              add_payload(state, payloads);
            }
            else
            {
              // Continue to match '+'
              add_state(label, state, new_states);
            }
            break;

          case detail::TopicMultiLevelWildcardChar:
            // Mactch using #
            add_payload(state, payloads);
            break;

          case detail::TopicLevelSeparatorChar:
            // Only add wildcard nodes at the beginning of a new level.
            add_wildcards(state, m_search_states);
            // Add pch to state list
            add_state(p_ch, state->get(p_ch), new_states);
            break;

          default:
            // Find p_ch.
            if(const auto &next_state = state->get(p_ch);
               next_state)
            {
              if(p_last)
              {
                // mqtt-v5.0-os
                // 4.7.1.2 Multi-level wildcard
                // 2956 ... if a Client subscribes to “sport/tennis/player1/#”, it would receive messages
                // 2957 published using these Topic Names:
                // 2958 • “sport/tennis/player1”
                // 2959 • “sport/tennis/player1/ranking
                // 2960 • “sport/tennis/player1/score/wimbledon”
                //if(detail::TopicLevelSeparatorChar == label)
                if(detail::TopicLevelSeparatorChar == p_ch)
                {
                  // Matched xxx/ so far, add wildcards to match xxx/+ or xxx/#
                  add_wildcards(next_state, m_search_states);
                }
                else
                {
                  // Matched xxx/ so far.
                  if(const auto & separator = next_state->get(detail::TopicLevelSeparatorChar);
                     separator)
                  {
                    // Add # wildcard to match xxx/#
                    // Check wildcards
                    add_state(detail::TopicMultiLevelWildcardChar,
                              separator->get(detail::TopicMultiLevelWildcardChar),
                              m_search_states);
                  }
                }
                add_payload(next_state, payloads);
              }
              else
              {
                // More to match...
                add_state(p_ch, next_state, new_states);
              }
            }
            break;
        }
      }

      std::swap(m_search_states, new_states);
      return !m_search_states.empty();
    }

    root_node_ptr m_root;
    queue m_search_states;
};

} // namespace detail

template<typename ValueType>
using Topics = yy_data::trie<char,
                             ValueType,
                             topics_detail::Query<ValueType>>;

} // namespace yafiyogi::yy_mqtt
