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

#include "yy_mqtt_constants.h"
#include "yy_mqtt_util.h"


namespace yafiyogi::yy_mqtt::tests {

class TestTopicUtil:
      public testing::Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(TestTopicUtil, TopicTrim)
{
  EXPECT_EQ(yy_mqtt::topic_trim("sport/finance"), "sport/finance");
  EXPECT_EQ(yy_mqtt::topic_trim("sport/finance/"), "sport/finance");
}

TEST_F(TestTopicUtil, TopicTokenize)
{
  EXPECT_EQ((TopicLevelsView{"abc"}), yy_mqtt::topic_tokenize_view("abc"));
  EXPECT_EQ((TopicLevelsView{"abc", ""}), yy_mqtt::topic_tokenize_view("abc/"));
  EXPECT_EQ((TopicLevelsView{"abc", "def"}), yy_mqtt::topic_tokenize_view("abc/def"));
  EXPECT_EQ((TopicLevelsView{"", "abc"}), yy_mqtt::topic_tokenize_view("/abc"));
  EXPECT_EQ((TopicLevelsView{"", "abc", ""}), yy_mqtt::topic_tokenize_view("/abc/"));
  EXPECT_EQ((TopicLevelsView{"", "abc", "def"}), yy_mqtt::topic_tokenize_view("/abc/def"));
}

TEST_F(TestTopicUtil, TestValidateSingleLevelWildcard)
{
  EXPECT_TRUE(yy_mqtt::topic_validate("+", yy_mqtt::TopicType::Filter));
  EXPECT_TRUE(yy_mqtt::topic_validate("+/tennis/#", yy_mqtt::TopicType::Filter));
  EXPECT_FALSE(yy_mqtt::topic_validate("sport+", yy_mqtt::TopicType::Filter));
  EXPECT_TRUE(yy_mqtt::topic_validate("sport/+/player1", yy_mqtt::TopicType::Filter));
  EXPECT_THROW(yy_mqtt::topic_validate("sport/+/player1", (yy_mqtt::TopicType)255), std::runtime_error);
}

TEST_F(TestTopicUtil, TestMatch)
{
  EXPECT_FALSE(yy_mqtt::topic_match("/finance/sport", "/finance"));
  EXPECT_FALSE(yy_mqtt::topic_match("sport", "finance"));
}

TEST_F(TestTopicUtil, TestMatchSingleLevelWildcard)
{
  EXPECT_TRUE(yy_mqtt::topic_match("+/+", "/finance"));
  EXPECT_FALSE(yy_mqtt::topic_match("+/+", "/finance/"));
  EXPECT_TRUE(yy_mqtt::topic_match("/+", "/finance"));
  EXPECT_FALSE(yy_mqtt::topic_match("/+", "/finance/"));
  EXPECT_FALSE(yy_mqtt::topic_match("+", "/finance"));
}

TEST_F(TestTopicUtil, TestValidateMultiLevelWildcard)
{
  EXPECT_TRUE(yy_mqtt::topic_validate("#", yy_mqtt::TopicType::Filter));
  EXPECT_TRUE(yy_mqtt::topic_validate("sport/tennis/#", yy_mqtt::TopicType::Filter));
  EXPECT_FALSE(yy_mqtt::topic_validate("sport/tennis#", yy_mqtt::TopicType::Filter));
  EXPECT_FALSE(yy_mqtt::topic_validate("sport/tennis/#/ranking", yy_mqtt::TopicType::Filter));
}

TEST_F(TestTopicUtil, TestMatchMultiLevelWildcard)
{
  EXPECT_TRUE(yy_mqtt::topic_match("sport/tennis/player1/#", "sport/tennis/player1"));
  EXPECT_TRUE(yy_mqtt::topic_match("sport/tennis/player1/#", "sport/tennis/player1/ranking"));
  EXPECT_TRUE(yy_mqtt::topic_match("sport/tennis/player1/#", "sport/tennis/player1/score/wimbledon"));
  EXPECT_TRUE(yy_mqtt::topic_match("sport/#", "sport"));
  EXPECT_TRUE(yy_mqtt::topic_match("#", "sport/tennis/player1"));
  EXPECT_TRUE(yy_mqtt::topic_match("#", "sport/tennis/player1/ranking"));
  EXPECT_TRUE(yy_mqtt::topic_match("#", "sport/tennis/player1/score/wimbledon"));
  EXPECT_TRUE(yy_mqtt::topic_match("#", "sport"));
}

TEST_F(TestTopicUtil, TestDollarMatch)
{
  EXPECT_FALSE(yy_mqtt::topic_match("#", "$sport/tennis/player1"));
  EXPECT_FALSE(yy_mqtt::topic_match("+/monlitor/Clients", "$SYS/monlitor/Clients"));
  EXPECT_TRUE(yy_mqtt::topic_match("$SYS/#", "$SYS/"));
  EXPECT_TRUE(yy_mqtt::topic_match("$SYS/#", "$SYS/monitor"));
  EXPECT_TRUE(yy_mqtt::topic_match("$SYS/monitor/+", "$SYS/monitor/Clients"));
}

} // namespace yafiyogi::yy_mqtt::tests
