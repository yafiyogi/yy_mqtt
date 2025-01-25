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

#include <cstddef>

#include <string_view>
#include <tuple>
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
    using size_type = std::size_t;
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
    using queue = std::vector<std::tuple<label_type, node_type *>>;
    using value_type = typename traits::value_type;
    using payloads_type = std::vector<value_type *>;
    using size_type = typename traits::size_type;

    constexpr explicit Query(root_node_ptr p_root) noexcept:
      m_root(std::move(p_root))
    {
    }

    Query() = delete;
    Query(const Query &) = delete;
    Query(Query &&) = delete;
    constexpr ~Query() noexcept = default;

    Query & operator=(const Query &) = delete;
    Query & operator=(Query &&) = delete;

    constexpr payloads_type find(std::string_view topic) noexcept
    {
      payloads_type payloads;

      m_search_states.clear();
      add_state(label_type{}, m_root.get(), m_search_states);

      if(!topic.empty())
      {
        if(mqtt_detail::TopicSysChar != topic[0])
        {
          add_wildcards(m_root.get(), m_search_states);
        }

        payloads.reserve(3);

        const auto max = topic.size();
        for(size_type idx = 0; idx < max; ++idx)
        {
          if(!next(topic[idx], idx == (max - 1), payloads))
          {
            break;
          }
        }
      }

      return payloads;
    }

  private:
    static constexpr void add_wildcards(node_type * p_node,
                                        queue & p_states_list) noexcept
    {
      add_node_state(mqtt_detail::TopicSingleLevelWildcardChar, p_node, p_states_list);
      add_node_state(mqtt_detail::TopicMultiLevelWildcardChar, p_node, p_states_list);
    }

    static constexpr void add_payload(node_type * p_node,
                                      payloads_type & p_payloads) noexcept
    {
      if(!p_node->empty())
      {
        p_payloads.emplace_back(&(p_node->value()));
      }
    }

    static constexpr node_type * get_state(node_type * node,
                                           const label_type p_label)
    {
      auto [state, found] = node->find_edge(p_label);

      return found ? state->m_node.get() : nullptr;
    }

    static constexpr void add_state(const label_type label,
                                    node_type * p_state,
                                    queue & p_states_list) noexcept
    {
      if( p_state)
      {
        p_states_list.emplace_back(label, p_state);
      }
    }

    static constexpr void add_node_state(const label_type p_label,
                                         node_type * node,
                                         queue & p_states_list)
    {
      node_type * state = get_state(node, p_label);
      if(state)
      {
        add_state(p_label, state, p_states_list);
      }
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
          case mqtt_detail::TopicSingleLevelWildcardChar:
            // Match using +
            // Re-add current node unless current ch is a level separator

            if(mqtt_detail::TopicLevelSeparatorChar == p_ch)
            {
              // Finished matching '+'
              if(node_type * separator = get_state(state, mqtt_detail::TopicLevelSeparatorChar);
                 nullptr != separator)
              {
                if(p_last)
                {
                  // Match xxx using xxx/+ or xxx/#
                  add_wildcards(separator, m_search_states);
                }
                else
                {
                  // Finished matching +, match / next.
                  add_state(mqtt_detail::TopicLevelSeparatorChar, separator, new_states);
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

          case mqtt_detail::TopicMultiLevelWildcardChar:
            // Mactch using #
            add_payload(state, payloads);
            break;

          case mqtt_detail::TopicLevelSeparatorChar:
            // Only add wildcard nodes at the beginning of a new level.
            add_wildcards(state, m_search_states);
            // Add pch to state list
            add_node_state(p_ch, state, new_states);
            break;

          default:
            // Find p_ch.
            if(node_type * next_state = get_state(state, p_ch);
               nullptr != next_state)
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
                if(mqtt_detail::TopicLevelSeparatorChar == p_ch)
                {
                  // Matched xxx/ so far, add wildcards to match xxx/+ or xxx/#
                  add_wildcards(next_state, m_search_states);
                }
                else
                {
                  // Matched xxx/ so far.
                  if(node_type * separator = get_state(next_state, mqtt_detail::TopicLevelSeparatorChar);
                     nullptr != separator)
                  {
                    // Add # wildcard to match xxx/#
                    // Check wildcards
                    add_node_state(mqtt_detail::TopicMultiLevelWildcardChar,
                                   separator,
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
using topics = yy_data::trie<char,
                             ValueType,
                             topics_detail::Query<ValueType>>;

} // namespace yafiyogi::yy_mqtt
