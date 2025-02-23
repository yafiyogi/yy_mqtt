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

#include <tuple>
#include <string_view>

#include "yy_cpp/yy_assert.h"
#include "yy_cpp/yy_fm_flat_trie_ptr.h"
#include "yy_cpp/yy_span.h"
#include "yy_cpp/yy_vector.h"

#include "yy_mqtt_constants.h"

namespace yafiyogi::yy_mqtt {
namespace flat_topics_detail {

template<typename TrieTraits>
class Query final
{
  public:
    using traits = TrieTraits;
    using label_type = typename traits::label_type;
    using node_type = typename traits::ptr_node_type;
    using node_ptr = typename traits::ptr_node_ptr;
    using value_type = typename traits::value_type;
    using value_ptr = typename traits::value_ptr;
    using trie_vector = typename traits::ptr_trie_vector;
    using data_vector = typename traits::data_vector;

    using queue = yy_quad::vector<std::tuple<node_ptr, label_type>>;
    using payloads_type = yy_quad::simple_vector<value_ptr>;
    using payloads_span_type = yy_quad::span<typename payloads_type::value_type>;

    constexpr explicit Query(trie_vector && p_nodes,
                             data_vector && p_data) noexcept:
      m_nodes(std::move(p_nodes)),
      m_data(std::move(p_data))
    {
      m_search_states.reserve(6);
      m_payloads.reserve(3);
    }

    Query() = delete;
    Query(const Query &) = delete;
    constexpr Query(Query &&) noexcept = default;
    constexpr ~Query() noexcept = default;

    Query & operator=(const Query &) = delete;
    constexpr Query & operator=(Query &&) noexcept = default;

    [[nodiscard]]
    constexpr payloads_span_type find(std::string_view topic) noexcept
    {
      m_search_states.clear(yy_quad::ClearAction::Keep);
      m_payloads.clear(yy_quad::ClearAction::Keep);

      add_state(label_type{}, node_ptr{m_nodes.data()}, m_search_states);

      if(!topic.empty())
      {
        if(mqtt_detail::TopicSysChar != topic[0])
        {
          add_wildcards(node_ptr{m_nodes.data()}, m_search_states);
        }

        const auto max = topic.size();
        for(size_type idx = 0; idx < max; ++idx)
        {
          if(!next(topic[idx], idx == (max - 1), m_payloads))
          {
            break;
          }
        }
      }

      return yy_quad::make_span(m_payloads);
    }

  private:
    constexpr void add_wildcards(node_ptr const p_node,
                                 queue & p_states_list) noexcept
    {
      add_node_state(mqtt_detail::TopicSingleLevelWildcardChar, p_node, p_states_list);
      add_node_state(mqtt_detail::TopicMultiLevelWildcardChar, p_node, p_states_list);
    }

    static constexpr void add_payload(node_ptr p_node,
                                      payloads_type & p_payloads) noexcept
    {
      if(!p_node->empty())
      {
        p_payloads.emplace_back(p_node->data());
      }
    }

    static constexpr node_ptr get_state(node_ptr p_node,
                                        const label_type p_label)
    {
      YY_ASSERT(p_node);

      node_ptr l_state{};
      auto l_next_state_do = [&l_state](auto edge_node, auto /* pos */) {
        l_state = *edge_node;
      };

      std::ignore = p_node->find_edge(l_next_state_do, p_label);

      return l_state;
    }

    static constexpr void add_state(const label_type label,
                                    node_ptr p_state,
                                    queue & p_states_list) noexcept
    {
      YY_ASSERT(p_state);

      p_states_list.emplace_back(p_state, label);
    }

    static constexpr void add_node_state(const label_type p_label,
                                         node_ptr p_node,
                                         queue & p_states_list)
    {
      YY_ASSERT(p_node);

      auto add_state_do = [&p_label, &p_states_list](node_ptr * edge_node, size_type /* pos */) {
        add_state(p_label, *edge_node, p_states_list);
      };

      std::ignore = p_node->find_edge(add_state_do, p_label);
    }

    constexpr bool next(const char p_ch,
                        const bool p_last,
                        payloads_type & payloads) noexcept
    {
      queue next_states;
      next_states.reserve(3);

      while(!m_search_states.empty())
      {
        auto [state, label] = m_search_states.front();
        m_search_states.erase(m_search_states.begin(), yy_quad::ClearAction::Keep);

        switch(label)
        {
          case mqtt_detail::TopicSingleLevelWildcardChar:
            // Match using +
            // Re-add current node unless current ch is a level separator

            if(mqtt_detail::TopicLevelSeparatorChar == p_ch)
            {
              // Finished matching '+'
              if(node_ptr separator{get_state(state, mqtt_detail::TopicLevelSeparatorChar)};
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
                  add_state(mqtt_detail::TopicLevelSeparatorChar, separator, next_states);
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
              add_state(label, state, next_states);
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
            add_node_state(p_ch, state, next_states);
            break;

          default:
            // Find p_ch.
            if(node_ptr next_state{get_state(state, p_ch)};
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
                if(mqtt_detail::TopicLevelSeparatorChar == p_ch)
                {
                  // Matched xxx/ so far, add wildcards to match xxx/+ or xxx/#
                  add_wildcards(next_state, m_search_states);
                }
                else
                {
                  // Matched xxx/ so far.
                  if(node_ptr separator{get_state(next_state, mqtt_detail::TopicLevelSeparatorChar)};
                     separator)
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
                add_state(p_ch, next_state, next_states);
              }
            }
            break;
        }
      }

      std::swap(m_search_states, next_states);
      return !m_search_states.empty();
    }

    trie_vector m_nodes;
    data_vector m_data;
    queue m_search_states;
    payloads_type m_payloads;

};

} // namespace flat_topics_detail

template<typename ValueType>
using flat_topics = yy_data::fm_flat_trie_ptr<char,
                                              ValueType,
                                              flat_topics_detail::Query>;

} // namespace yafiyogi::yy_mqtt
