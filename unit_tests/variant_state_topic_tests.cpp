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

#include "fmt/format.h"
#include "gtest/gtest.h"

#include "yy_cpp/yy_tokenizer.h"

#include "yy_mqtt_constants.h"
#include "yy_mqtt_util.h"
#include "yy_mqtt_variant_state_topics.h"

namespace yafiyogi::yy_mqtt::tests {

class TestVariantStateTopics:
      public testing::Test
{
  public:
    using variant_state_topics = yafiyogi::yy_mqtt::variant_state_topics<int>;
    using Automaton = variant_state_topics::automaton_type;
    using Values = std::vector<Automaton::value_type>;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    bool test_topic(const std::vector<std::tuple<std::string_view, int>> & p_filters,
                    const std::string_view p_topic,
                    Values && p_values)
    {
      variant_state_topics l_topics{};

      for(auto & filter : p_filters)
      {
        auto & [topic, value] = filter;
        l_topics.add(topic, value);
      }

      auto count = p_values.size();
      auto value_ptr = p_values.begin();
      const auto end_ptr = p_values.end();

      auto test_fn = [&value_ptr, end_ptr, &count](const auto payload) {
        if(end_ptr != value_ptr)
        {
          if(*payload == *value_ptr)
          {
            --count;
          }
          else
          {
            fmt::print("payload=[{}] expected=[{}]\n", *payload, *value_ptr);
          }
          ++value_ptr;
        }
      };

      auto automaton = l_topics.create_automaton();
      auto payloads = automaton.find(p_topic);

      for(const auto & payload : payloads)
      {
        test_fn(payload);
      }

      return (end_ptr == value_ptr) && (0 == count) && (payloads.size() == p_values.size());
    }
};

TEST_F(TestVariantStateTopics, TestMatchSingleLevelWildcard)
{
  EXPECT_TRUE(test_topic({{"sport/+", 111}}, "sport", Values{}));
  EXPECT_TRUE(test_topic({{"sport/+", 222}}, "sport/", Values{222}));
  EXPECT_TRUE(test_topic({{"+/+", 333}}, "/finance", Values{333}));
  EXPECT_TRUE(test_topic({{"+/+", 444}}, "/finance/", Values{444}));
  EXPECT_TRUE(test_topic({{"/+", 555}}, "/finance", Values{555}));
  EXPECT_TRUE(test_topic({{"/+", 666}},"/finance/", Values{666}));
  EXPECT_TRUE(test_topic({{"+", 777}}, "/finance", Values{}));
}

TEST_F(TestVariantStateTopics, TestMatchMultiLevelWildcard)
{
  EXPECT_TRUE(test_topic({{"sport/tennis/player1/#", 111}}, "sport/tennis/player1", Values{111}));
  EXPECT_TRUE(test_topic({{"sport/tennis/player1/#", 222}}, "sport/tennis/player1/", Values{222}));
  EXPECT_TRUE(test_topic({{"sport/tennis/player1/#", 333}}, "sport/tennis/player1/ranking", Values{333}));
  EXPECT_TRUE(test_topic({{"sport/tennis/player1/#", 444}}, "sport/tennis/player1/score/wimbledon", Values{444}));
  EXPECT_TRUE(test_topic({{"sport/#", 555}}, "sport", Values{555}));
}

TEST_F(TestVariantStateTopics, TestDollarMatch)
{
  EXPECT_TRUE(test_topic({{"#", 111}}, "$sport/tennis/player1", Values{}));
  EXPECT_TRUE(test_topic({{"+/monlitor/Clients", 222}}, "$SYS/monlitor/Clients", Values{}));
  EXPECT_TRUE(test_topic({{"$SYS/#", 333}}, "$SYS/", Values{333}));
  EXPECT_TRUE(test_topic({{"$SYS/#", 444}}, "$SYS/monitor", Values{444}));
  EXPECT_TRUE(test_topic({{"$SYS/monitor/+", 555}}, "$SYS/monitor/Clients", Values{555}));
}

TEST_F(TestVariantStateTopics, TestTrieMatch)
{
  EXPECT_TRUE(test_topic({{"/+", 111},{"/+/+", 112}}, "/roofer", Values{111}));
  EXPECT_TRUE(test_topic({{"/+", 222},{"/+/+", 223}}, "/roofer/", Values{222, 223}));
  EXPECT_TRUE(test_topic({{"/+", 333},{"/+/#", 334}}, "/roofer", Values{333, 334}));
  EXPECT_TRUE(test_topic({{"/+", 444}}, "/roofer/", Values{444}));
  EXPECT_TRUE(test_topic({{"/+/#", 555}}, "/roofer/", Values{555}));
  EXPECT_TRUE(test_topic({{"/+", 666},{"/+/#", 667}}, "/roofer/", Values{666, 667}));
  EXPECT_TRUE(test_topic({{"/+/+", 777},{"/+/#", 778}}, "/roofer/", Values({777, 778})));
}

// Based on https://github.com/eclipse/mosquitto/blob/v2.0.18/test/unit/util_topic_test.c
TEST_F(TestVariantStateTopics, TestMosquittoValidMatching)
{
  EXPECT_TRUE(test_topic({{"foo/#", 111}}, "foo/", Values{111}));
  EXPECT_TRUE(test_topic({{"foo/#", 222}}, "foo", Values{222}));
  EXPECT_TRUE(test_topic({{"foo//bar", 333}}, "foo//bar", Values{333}));
  EXPECT_TRUE(test_topic({{"foo//+", 444}}, "foo//bar", Values{444}));
  EXPECT_TRUE(test_topic({{"foo/+/+/baz", 555}}, "foo///baz", Values{555}));
  EXPECT_TRUE(test_topic({{"foo/bar/+", 666}}, "foo/bar/", Values{666}));
  EXPECT_TRUE(test_topic({{"foo/bar", 777}}, "foo/bar", Values{777}));
  EXPECT_TRUE(test_topic({{"foo/+", 888}}, "foo/bar", Values{888}));
  EXPECT_TRUE(test_topic({{"foo/+/baz", 999}}, "foo/bar/baz", Values{999}));
  EXPECT_TRUE(test_topic({{"A/B/+/#", 1111}}, "A/B/B/C", Values{1111}));
  EXPECT_TRUE(test_topic({{"foo/+/#", 2222}}, "foo/bar/baz", Values{2222}));
  EXPECT_TRUE(test_topic({{"foo/+/#", 3333}}, "foo/bar", Values{3333}));
  EXPECT_TRUE(test_topic({{"#", 4444}}, "foo/bar/baz", Values{4444}));
  EXPECT_TRUE(test_topic({{"#", 5555}}, "foo/bar/baz", Values{5555}));
  EXPECT_TRUE(test_topic({{"#", 6666}}, "/foo/bar", Values{6666}));
  EXPECT_TRUE(test_topic({{"/#", 7777}}, "/foo/bar", Values{7777}));
}

// Based on https://github.com/eclipse/mosquitto/blob/v2.0.18/test/unit/util_topic_test.c
TEST_F(TestVariantStateTopics, TestMosquittoValidNoMatching)
{
  EXPECT_TRUE(test_topic({{"test/6/#", 111}}, "test/3", Values{}));
  EXPECT_TRUE(test_topic({{"foo/bar", 222}}, "foo", Values{}));
  EXPECT_TRUE(test_topic({{"foo/+", 333}}, "foo/bar/baz", Values{}));
  EXPECT_TRUE(test_topic({{"foo/+/baz", 444}}, "foo/bar/bar", Values{}));
  EXPECT_TRUE(test_topic({{"foo/+/#", 555}}, "fo2/bar/baz", Values{}));
  EXPECT_TRUE(test_topic({{"/#", 666}}, "foo/bar", Values{}));
  EXPECT_TRUE(test_topic({{"#", 777}}, "$SYS/bar", Values{}));
  EXPECT_TRUE(test_topic({{"$BOB/bar", 888}}, "$SYS/bar", Values{}));
}

} // namespace yafiyogi::yy_mqtt::tests
