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

#include <string_view>

#include "benchmark/benchmark.h"

#include "yy_mqtt_topics.h"
#include "yy_mqtt_flat_topics.h"
#include "yy_mqtt_fast_topics.h"
#include "yy_mqtt_faster_topics.h"


using Topics = yafiyogi::yy_mqtt::topics<int>;
using FlatTopics = yafiyogi::yy_mqtt::flat_topics<int>;
using FastTopics = yafiyogi::yy_mqtt::fast_topics<int>;
using FasterTopics = yafiyogi::yy_mqtt::faster_topics<int>;

namespace yafiyogi::benchmark {

struct TopicsFixtureType:
      public ::benchmark::Fixture
{
  public:
    TopicsFixtureType();
    void SetUp(const ::benchmark::State &) override;

    static std::string_view query(size_t idx);
    static size_t query_size();
    static size_t topics_size();

    static Topics m_topics;
    static FlatTopics m_flat_topics;
    static FastTopics m_fast_topics;
    static FasterTopics m_faster_topics;
};



} // namespace yafiyogi::benchmark
