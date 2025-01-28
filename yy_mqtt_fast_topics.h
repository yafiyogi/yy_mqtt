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

#include <cstdint>
#include <string_view>

#include "yy_cpp/yy_assert.h"
#include "yy_cpp/yy_fm_flat_trie_ptr.h"
#include "yy_cpp//yy_ref_traits.h"
#include "yy_cpp/yy_span.h"
#include "yy_cpp/yy_vector.h"

#include "yy_mqtt_constants.h"

namespace yafiyogi::yy_mqtt {
namespace fast_topics_detail {

template<typename TrieTraits>
class Query final
{
  public:
    using traits = TrieTraits;
    using label_type = typename traits::label_type;
    using node_type = typename traits::ptr_node_type;
    using node_ptr = typename traits::ptr_node_ptr;
    using value_type = typename traits::value_type;
    using trie_vector = typename traits::ptr_trie_vector;
    using trie_iterator = trie_vector::iterator;
    using data_vector = typename traits::data_vector;
    using topic_type = yy_quad::const_span<label_type>;
    using topic_l_value_ref = typename yy_traits::ref_traits<topic_type>::l_value_ref;
    using payloads_type = yy_quad::simple_vector<value_type *>;
    using payloads_span_type = yy_quad::span<typename payloads_type::value_type>;
    enum class search_type:uint8_t {Literal, SingleLevel, MultiLevel};
    struct state_type final
    {
        topic_type topic{};
        node_type * state = nullptr;
        search_type search = search_type::Literal;
    };
    using queue = yy_quad::vector<state_type>;

    constexpr explicit Query(trie_vector && p_nodes,
                             data_vector && p_data) noexcept:
      m_nodes(std::move(p_nodes)),
      m_data(std::move(p_data))
    {
      m_search_states.reserve(6);
      m_payloads.reserve(3);
    }

    constexpr Query() noexcept = default;
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

      if(!topic.empty())
      {
        find_span(yy_quad::make_const_span(topic));
      }

      return yy_quad::make_span(m_payloads);
    }

  private:
    static constexpr void add_sub_state(const label_type p_label,
                                        topic_type p_topic,
                                        search_type p_type,
                                        node_type * p_state,
                                        queue & p_states_list)
    {
      YY_ASSERT(p_state);

      auto sub_state_do = [p_topic, p_type, &p_states_list]
                          (node_type ** edge_node, size_type /* pos */) {
        p_states_list.emplace_back(p_topic, *edge_node, p_type);
      };

      std::ignore = p_state->find_edge(sub_state_do, p_label);
    }

    static constexpr void add_wildcards(node_ptr p_node,
                                        topic_type topic,
                                        queue & p_states_list) noexcept
    {
      YY_ASSERT(p_node);

      add_sub_state(mqtt_detail::TopicSingleLevelWildcardChar, topic, search_type::SingleLevel, p_node, p_states_list);
      add_sub_state(mqtt_detail::TopicMultiLevelWildcardChar, topic, search_type::MultiLevel, p_node, p_states_list);
    }

    static constexpr void add_payload(node_ptr p_node,
                                      payloads_type & p_payloads) noexcept
    {
      YY_ASSERT(p_node);

      if(!p_node->empty())
      {
        p_payloads.emplace_back(p_node->data());
      }
    }

    constexpr void find_span(topic_type p_topic) noexcept
    {
      auto do_add_separator_wildcards = [this](node_type ** separator_node, size_type) {
        add_wildcards(*separator_node, topic_type{}, m_search_states);
      };

      m_search_states.emplace_back(p_topic, m_nodes.data(), search_type::Literal);
      add_wildcards(m_nodes.data(), p_topic, m_search_states);

      while(!m_search_states.empty())
      {
        auto [search_topic, state, type] = m_search_states.front();
        m_search_states.erase(m_search_states.begin(), yy_quad::ClearAction::Keep);

        switch(type)
        {
          case search_type::Literal:
          {
            auto next_state_do = [&state](node_type ** edge_node, size_type) {
              state = *edge_node;
            };

            bool found_separator = false;
            bool found = true;
            while(!search_topic.empty())
            {
              const auto topic_part = search_topic[0];

              found = state->find_edge(next_state_do,
                                       topic_part);
              if(!found)
              {
                break;
              }

              search_topic.inc_begin();

              found_separator = mqtt_detail::TopicLevelSeparatorChar == topic_part;
              if(found_separator)
              {
                // Topic is 'abc/cde/', try to match 'abc/cde/+' & 'abc/cde/#'
                add_wildcards(state, search_topic, m_search_states);
              }
            }

            if(found && search_topic.empty())
            {
              // Topic is 'abc/cde' or 'abc/cde/', add any payloads.
              add_payload(state, m_payloads);

              if(!found_separator)
              {
                // In case of 'abc/cde', try to match 'abc/cde/+' & 'abc/cde/#'
                std::ignore = state->find_edge(do_add_separator_wildcards,
                                              mqtt_detail::TopicLevelSeparatorChar);
              }
            }
            break;
          }

          case search_type::SingleLevel:
            while(!search_topic.empty())
            {
              if(mqtt_detail::TopicLevelSeparatorChar == search_topic[0])
              {
                // End of '+' match.
                if(search_topic.size() == 1)
                {
                  // Topic is 'abc/+/', so add payloads.
                  add_payload(state, m_payloads);
                }
                break;
              }
              search_topic.inc_begin();
            }

            if(search_topic.empty())
            {
              // Topic is 'abc/+', so add payloads.
              add_payload(state, m_payloads);

              std::ignore = state->find_edge(do_add_separator_wildcards,
                                            mqtt_detail::TopicLevelSeparatorChar);
            }
            else
            {
              // Topic is 'abc/+/...' so ...
              auto separator_do = [&search_topic, this](node_type ** edge_node, size_type) {
                auto separator_state = *edge_node;
                search_topic.inc_begin();
                // ... try to match 'abc/+/cde
                if(!search_topic.empty())
                {
                  m_search_states.emplace_back(search_topic, separator_state, search_type::Literal);
                }
                else
                {
                  add_payload(separator_state, m_payloads);
                }
                // ... try to match 'abc/+/+' and 'abc/+/#'
                add_wildcards(separator_state, search_topic, m_search_states);
              };

              std::ignore = state->find_edge(separator_do, search_topic[0]);
            }
            break;

          case search_type::MultiLevel:
            add_payload(state, m_payloads);
            break;
        }
      }
    }

    trie_vector m_nodes;
    data_vector m_data;
    queue m_search_states;
    payloads_type m_payloads;
};

} // namespace fast_topics_detail

template<typename ValueType>
using fast_topics = yy_data::fm_flat_trie_ptr<char,
                                              ValueType,
                                              fast_topics_detail::Query>;

} // namespace yafiyogi::yy_mqtt
