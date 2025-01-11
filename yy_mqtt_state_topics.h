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
namespace state_topics_detail {

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
    using size_type = typename traits::size_type;
    using trie_vector = typename traits::ptr_trie_vector;
    using trie_iterator = trie_vector::iterator;
    using data_vector = typename traits::data_vector;
    using payloads_type = yy_quad::simple_vector<value_ptr>;
    using payloads_span_type = yy_quad::span<typename payloads_type::value_type>;
    using tokenizer_type = typename traits::tokenizer_type;
    using topic_type = typename tokenizer_type::token_type;

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
    class state_type;
    using queue = yy_quad::vector<state_type, yy_data::ClearAction::Keep>;
    using find_fn = void (*)(topic_type /* p_topic */,
                             node_ptr /* p_state */,
                             queue & /* p_search_states */,
                             payloads_type & /* p_payloads */) noexcept;

    class state_type
    {
      public:
        constexpr state_type(topic_type p_topic,
                             node_ptr p_state,
                             find_fn p_find) noexcept:
          m_topic(p_topic),
          m_state(p_state),
          m_find(p_find)
        {
          YY_ASSERT(p_find);
        }

        constexpr state_type() noexcept = default;
        state_type(const state_type &) = delete;
        constexpr state_type(state_type &&) noexcept = default;
        constexpr ~state_type() noexcept = default;

        constexpr state_type & operator=(const state_type &) noexcept = default;
        constexpr state_type & operator=(state_type &&) noexcept = default;

        constexpr void operator()(queue & p_search_states,
                                  payloads_type & p_payloads) noexcept
        {
          m_find(m_topic, m_state, p_search_states, p_payloads);
        }

      private:
        topic_type m_topic{};
        node_ptr m_state = nullptr;
        find_fn m_find = &null_find;
    };

    static constexpr void add_sub_state(const topic_type p_label,
                                        topic_type p_topic,
                                        node_ptr p_node,
                                        find_fn p_find,
                                        queue & p_search_states) noexcept
    {
      auto sub_state_do = [p_topic, p_find, &p_search_states]
                          (node_ptr * edge_node, size_type /* pos */) {
        p_search_states.emplace_back(p_topic, *edge_node, p_find);
      };

      std::ignore = p_node->find_edge(sub_state_do, p_label);
    }

    static constexpr const topic_type single_level_wildcard{yy_quad::make_const_span(mqtt_detail::TopicSingleLevelWildcard)};
    static constexpr const topic_type multi_level_wildcard{yy_quad::make_const_span(mqtt_detail::TopicMultiLevelWildcard)};

    static constexpr void add_payload(node_ptr p_node,
                                      payloads_type & p_payloads) noexcept
    {
      if(!p_node->empty())
      {
        p_payloads.emplace_back(p_node->data());
      }
    }

    static constexpr void null_find(topic_type /* p_topic */,
                                    node_ptr /* p_state */,
                                    queue & /* p_search_states */,
                                    payloads_type & /* p_payloads */) noexcept
    {
    }

    static constexpr void literal_find(topic_type p_topic,
                                       node_ptr p_state,
                                       queue & p_search_states,
                                       payloads_type & p_payloads) noexcept
    {
      auto next_state_do = [&p_state](node_ptr * edge_node, size_type) {
        p_state = *edge_node;
      };

      tokenizer_type topic_tokens{p_topic};

      while(!topic_tokens.empty())
      {
        if(const auto level{topic_tokens.scan()};
           !p_state->find_edge(next_state_do, level))
        {
          return;
        }

        auto rest_topic{topic_tokens.source()};
        if(topic_tokens.has_more())
        {
          // mqtt-v5.0 4.7.1.3 Single-level wildcard
          // 2979: "sport/+” does not match “sport” but it does match “sport/”.
          // Topic is 'abc/cde/', try to match 'abc/cde/+'.
          add_sub_state(single_level_wildcard, rest_topic, p_state, &single_level_find, p_search_states);
        }
        // Topic is 'abc/cde' or 'abc/cde/', so try to match 'abc/cde/#'
        add_sub_state(multi_level_wildcard, rest_topic, p_state, &multi_level_find, p_search_states);
      }

      // Topic is 'abc/cde' or 'abc/cde/', add any payloads.
      add_payload(p_state, p_payloads);
    }

    static constexpr void single_level_find(topic_type p_topic,
                                            node_ptr p_state,
                                            queue & p_search_states,
                                            payloads_type & p_payloads) noexcept
    {
      tokenizer_type topic_tokens{p_topic};
      std::ignore = topic_tokens.scan();

      auto rest_topic{topic_tokens.source()};
      if(rest_topic.empty())
      {
        // Topic is 'abc/+', so add payloads.
        add_payload(p_state, p_payloads);
      }
      else
      {
        // Try to match 'abc/+/cde
        p_search_states.emplace_back(rest_topic, p_state, literal_find);
      }

      // Try to match 'abc/+/+' and 'abc/+/#'
      //        auto rest_topic{topic_tokens.source()};
      if(topic_tokens.has_more())
      {
        // mqtt-v5.0 4.7.1.3 Single-level wildcard
        // 2979: "sport/+” does not match “sport” but it does match “sport/”.
        // Topic is 'abc/cde/', try to match 'abc/cde/+'.
        add_sub_state(single_level_wildcard, rest_topic, p_state, &single_level_find, p_search_states);
      }
      // Topic is 'abc/cde' or 'abc/cde/', so try to match 'abc/cde/#'
      add_sub_state(multi_level_wildcard, rest_topic, p_state, &multi_level_find, p_search_states);
    }

    static constexpr void multi_level_find(topic_type /* p_topic */,
                                           node_ptr p_state,
                                           queue & /* p_search_states */,
                                           payloads_type & p_payloads) noexcept
    {
      add_payload(p_state, p_payloads);
    }

    constexpr void find_span(topic_type p_topic) noexcept
    {
      m_search_states.emplace_back(p_topic, m_nodes.data(), literal_find);
      if(mqtt_detail::TopicSysChar != p_topic[0])
      {
        add_sub_state(single_level_wildcard, p_topic, m_nodes.data(), &single_level_find, m_search_states);
        add_sub_state(multi_level_wildcard, p_topic, m_nodes.data(), &multi_level_find, m_search_states);
      }

      while(!m_search_states.empty())
      {
        auto & find = m_search_states.front();

        find(m_search_states, m_payloads);

        m_search_states.pop_front(yy_quad::ClearAction::Keep);
      }
    }

    trie_vector m_nodes{};
    data_vector m_data{};
    queue m_search_states{};
    payloads_type m_payloads{};
};

template<typename LabelType>
using tokenizer_type = yy_trie::label_word_tokenizer<LabelType,
                                                     mqtt_detail::TopicLevelSeparatorChar,
                                                     yy_util::tokenizer>;


} // namespace state_topics_detail


template<typename ValueType>
using state_topics = yy_data::fm_flat_trie_ptr<std::string,
                                               ValueType,
                                               state_topics_detail::Query,
                                               state_topics_detail::tokenizer_type>;

} // namespace yafiyogi::yy_mqtt
