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

#include <string>
#include <string_view>

#include "yy_cpp/yy_assert.h"
#include "yy_cpp/yy_fm_flat_trie_ptr.h"
#include "yy_cpp/yy_ref_traits.h"
#include "yy_cpp/yy_span.h"
#include "yy_cpp/yy_tokenizer.h"
#include "yy_cpp/yy_vector.h"

#include "yy_mqtt_constants.h"

namespace yafiyogi::yy_mqtt {
namespace faster_topics_detail {

template<typename LabelType,
         typename ValueType>
class Query final
{
  public:
    using traits = yy_data::fm_flat_trie_ptr_detail::trie_ptr_traits<LabelType, ValueType>;
    using label_type = typename traits::label_type;
    using node_type = typename traits::node_type;
    using value_type = typename traits::value_type;
    using size_type = typename traits::size_type;
    using trie_vector = typename traits::trie_vector;
    using data_vector = typename traits::data_vector;
    using topic_type = yy_quad::const_span<std::string_view::value_type>;
    using topic_l_value_ref = typename yy_traits::ref_traits<topic_type>::l_value_ref;
    using payloads_type = yy_quad::simple_vector<value_type *>;
    using payloads_span_type = yy_quad::span<typename payloads_type::value_type>;
    enum class search_type:uint8_t {Literal, SingleLevelWild, MultiLevelWild};
    using tokenizer = yy_util::tokenizer<std::string_view::value_type>;

    struct state_type
    {
        topic_type topic{};
        node_type * state{};
        search_type search = search_type::Literal;
    };
    using queue = yy_quad::vector<state_type>;

    constexpr explicit Query(trie_vector && p_nodes,
                             data_vector && p_data) noexcept:
      m_nodes(std::move(p_nodes)),
      m_data(std::move(p_data))
    {
      m_search_states.reserve(8);
      m_payloads.reserve(3);
    }

    constexpr Query() noexcept = default;
    Query(const Query &) = delete;
    constexpr Query(Query &&) noexcept = default;

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
    static constexpr void add_sub_state(const std::string_view p_label,
                                        topic_type p_topic,
                                        search_type p_type,
                                        node_type * p_state,
                                        queue & p_states_list)
    {
      YY_ASSERT(p_state);

      auto sub_state_do = [p_label, p_topic, p_type, &p_states_list]
                          (node_type ** edge_node, size_type /* pos */) {
        p_states_list.emplace_back(state_type{p_topic, *edge_node, p_type});
      };

      std::ignore = p_state->find_edge(sub_state_do, p_label);
    }

    static constexpr void add_wildcards(node_type * p_node,
                                        topic_type p_topic,
                                        queue & p_states_list) noexcept
    {
      YY_ASSERT(p_node);

      add_sub_state(mqtt_detail::TopicSingleLevelWildcard, p_topic, search_type::SingleLevelWild, p_node, p_states_list);
      add_sub_state(mqtt_detail::TopicMultiLevelWildcard, p_topic, search_type::MultiLevelWild, p_node, p_states_list);
    }

    static constexpr bool add_payload(node_type * p_node,
                                      payloads_type & p_payloads) noexcept
    {
      YY_ASSERT(p_node);

      bool add = !p_node->empty();

      if(add)
      {
        p_payloads.emplace_back(p_node->data());
      }

      return add;
    }

    constexpr void find_span(topic_type p_topic) noexcept
    {
      m_search_states.emplace_back(state_type{p_topic, m_nodes.begin(), search_type::Literal});
      add_wildcards(m_nodes.begin(), p_topic, m_search_states);

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

            tokenizer topic_tokens{search_topic, mqtt_detail::TopicLevelSeparatorChar};

            bool found = false;
            while(!topic_tokens.empty())
            {
              const auto level = topic_tokens.scan();

              found = state->find_edge(next_state_do, level);

              if(!found)
              {
                break;
              }

              // Topic is 'abc/cde/', try to match 'abc/cde/+' & 'abc/cde/#'
              add_wildcards(state, topic_tokens.source(), m_search_states);
            }

            if(found)
            {
              // Topic is 'abc/cde' or 'abc/cde/', add any payloads.
              add_payload(state, m_payloads);
            }
            break;
          }

          case search_type::SingleLevelWild:
          {
            tokenizer topic_tokens{search_topic, mqtt_detail::TopicLevelSeparatorChar};
            std::ignore = topic_tokens.scan();

            auto topic{topic_tokens.source()};
            if(topic.empty())
            {
              // Topic is 'abc/+', so add payloads.
              add_payload(state, m_payloads);
            }
            else
            {
              // Try to match 'abc/+/cde
              m_search_states.emplace_back(state_type{topic, state, search_type::Literal});
            }

            // Try to match 'abc/+/+' and 'abc/+/#'
            add_wildcards(state, topic, m_search_states);
            break;
          }

          case search_type::MultiLevelWild:
            add_payload(state, m_payloads);
            break;
        }
      }
    }

    trie_vector m_nodes{};
    data_vector m_data{};
    queue m_search_states{};
    payloads_type m_payloads{};
};

} // namespace faster_topics_detail

template<typename ValueType>
using faster_topics = yy_data::fm_flat_trie_ptr<std::string,
                                                ValueType,
                                                faster_topics_detail::Query>;
template<typename ValueType>
constexpr auto faster_topics_add(faster_topics<ValueType> & topics,
                                 std::string_view topic,
                                 const ValueType & value)
{
  yy_quad::simple_vector<std::string> topic_levels;
  yy_util::tokenizer<std::string::value_type> tokenizer{yy_quad::make_const_span(topic),
                                                        mqtt_detail::TopicLevelSeparatorChar};

  while(!tokenizer.empty())
  {
    auto token = tokenizer.scan();
    topic_levels.emplace_back(std::string{token.begin(), token.end()});
  }

  return topics.add(topic_levels, value);
}

} // namespace yafiyogi::yy_mqtt
