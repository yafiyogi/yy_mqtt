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

TEST_F(TestTopicUtil, TopicTokenizeView)
{
  EXPECT_EQ((TopicLevelsView{"abc"}), yy_mqtt::topic_tokenize_view("abc"));
  EXPECT_EQ((TopicLevelsView{"abc", ""}), yy_mqtt::topic_tokenize_view("abc/"));
  EXPECT_EQ((TopicLevelsView{"abc", "def"}), yy_mqtt::topic_tokenize_view("abc/def"));
  EXPECT_EQ((TopicLevelsView{"", "abc"}), yy_mqtt::topic_tokenize_view("/abc"));
  EXPECT_EQ((TopicLevelsView{"", "abc", ""}), yy_mqtt::topic_tokenize_view("/abc/"));
  EXPECT_EQ((TopicLevelsView{"", "abc", "def"}), yy_mqtt::topic_tokenize_view("/abc/def"));
}

TEST_F(TestTopicUtil, TopicTokenize)
{
  EXPECT_EQ((TopicLevels{"abc"}), yy_mqtt::topic_tokenize("abc"));
  EXPECT_EQ((TopicLevels{"abc", ""}), yy_mqtt::topic_tokenize("abc/"));
  EXPECT_EQ((TopicLevels{"abc", "def"}), yy_mqtt::topic_tokenize("abc/def"));
  EXPECT_EQ((TopicLevels{"", "abc"}), yy_mqtt::topic_tokenize("/abc"));
  EXPECT_EQ((TopicLevels{"", "abc", ""}), yy_mqtt::topic_tokenize("/abc/"));
  EXPECT_EQ((TopicLevels{"", "abc", "def"}), yy_mqtt::topic_tokenize("/abc/def"));
}

TEST_F(TestTopicUtil, TestValidateSingleLevelWildcard)
{
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate("+", yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate("+/tennis/#", yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Invalid, yy_mqtt::topic_validate("sport+", yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate("sport/+/player1", yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::BadParam, yy_mqtt::topic_validate("sport/+/player1", (yy_mqtt::TopicType)255));

  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("+"), yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("+/tennis/#"), yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Invalid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("sport+"), yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("sport/+/player1"), yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::BadParam, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("sport/+/player1"), (yy_mqtt::TopicType)255));
}

TEST_F(TestTopicUtil, TestMatch)
{
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match("/finance/sport", "/finance"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match("sport", "finance"));

  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("/finance/sport"), yy_mqtt::topic_tokenize_view("/finance")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("sport"), yy_mqtt::topic_tokenize_view("finance")));
}

TEST_F(TestTopicUtil, TestMatchSingleLevelWildcard)
{
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("+/+", "/finance"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("+/+", "/finance/"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("/+", "/finance"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("/+", "/finance/"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match("+", "/finance"));

  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("+/+"), yy_mqtt::topic_tokenize_view("/finance")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("+/+"), yy_mqtt::topic_tokenize_view("/finance/")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("/+"), yy_mqtt::topic_tokenize_view("/finance")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("/+"), yy_mqtt::topic_tokenize_view("/finance/")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("+"), yy_mqtt::topic_tokenize_view("/finance")));
}

TEST_F(TestTopicUtil, TestValidateMultiLevelWildcard)
{
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate("#", yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate("sport/tennis/#", yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Invalid, yy_mqtt::topic_validate("sport/tennis#", yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Invalid, yy_mqtt::topic_validate("sport/tennis/#/ranking", yy_mqtt::TopicType::Filter));

  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("#"), yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Valid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("sport/tennis/#"), yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Invalid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("sport/tennis#"), yy_mqtt::TopicType::Filter));
  EXPECT_EQ(yy_mqtt::TopicValidStatus::Invalid, yy_mqtt::topic_validate(yy_mqtt::topic_tokenize_view("sport/tennis/#/ranking"), yy_mqtt::TopicType::Filter));
}

TEST_F(TestTopicUtil, TestMatchMultiLevelWildcard)
{
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("sport/tennis/player1/#", "sport/tennis/player1"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("sport/tennis/player1/#", "sport/tennis/player1/ranking"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("sport/tennis/player1/#", "sport/tennis/player1/score/wimbledon"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("sport/#", "sport"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("#", "sport/tennis/player1"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("#", "sport/tennis/player1/ranking"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("#", "sport/tennis/player1/score/wimbledon"));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match("#", "sport"));

  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("sport/tennis/player1/#"), yy_mqtt::topic_tokenize_view("sport/tennis/player1")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("sport/tennis/player1/#"), yy_mqtt::topic_tokenize_view("sport/tennis/player1/ranking")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("sport/tennis/player1/#"), yy_mqtt::topic_tokenize_view("sport/tennis/player1/score/wimbledon")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("sport/#"), yy_mqtt::topic_tokenize_view("sport")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("#"), yy_mqtt::topic_tokenize_view("sport/tennis/player1")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("#"), yy_mqtt::topic_tokenize_view("sport/tennis/player1/ranking")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("#"), yy_mqtt::topic_tokenize_view("sport/tennis/player1/score/wimbledon")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("#"), yy_mqtt::topic_tokenize_view("sport")));
}

TEST_F(TestTopicUtil, TestDollarMatch)
{
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("#"), yy_mqtt::topic_tokenize_view("$sport/tennis/player1")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Fail, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("+/monlitor/Clients"), yy_mqtt::topic_tokenize_view("$SYS/monlitor/Clients")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("$SYS/#"), yy_mqtt::topic_tokenize_view("$SYS/")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("$SYS/#"), yy_mqtt::topic_tokenize_view("$SYS/monitor")));
  EXPECT_EQ(yy_mqtt::TopicMatchStatus::Match, yy_mqtt::topic_match(yy_mqtt::topic_tokenize_view("$SYS/monitor/+"), yy_mqtt::topic_tokenize_view("$SYS/monitor/Clients")));
}

} // namespace yafiyogi::yy_mqtt::tests
