// Copyright 2020 Andrey Golubev
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// ANDANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <count_until.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <list>
#include <string>
#include <vector>

TEST(count_until_tests, empty_collection) {
    const std::vector<int> collection{};
    const auto always_false = [](int) { return false; };
    EXPECT_EQ(0, clsc::count_until(collection.cbegin(), collection.cend(), always_false));
}

// TODO: add type tests?
TEST(count_until_tests, simple_condition) {
    const std::list<int> collection{1, 2, 3, 4, 5};
    const auto value_in_range = [](int e) { return e > 2 && e < 4; };
    auto expected = std::distance(
        collection.cbegin(), std::find_if(collection.cbegin(), collection.cend(), value_in_range));
    auto counted = clsc::count_until(collection.cbegin(), collection.cend(), value_in_range);
    EXPECT_EQ(expected, counted);
}

TEST(count_until_tests, always_false_condition) {
    const std::vector<int> collection{1, 2, 3, 4, 5};
    const auto always_false = [](int) { return false; };
    EXPECT_EQ(collection.size(),
              clsc::count_until(collection.cbegin(), collection.cend(), always_false));
}

TEST(count_until_tests, date_time) {
    const std::string date_time("2019-15-11 15.10 UTC+00:00");
    int date_count = 0;
    {
        const auto not_date_symbols = [](char c) {
            return !(c == '-' || std::isdigit(static_cast<unsigned char>(c)));
        };
        date_count = clsc::count_until(date_time.cbegin(), date_time.cend(), not_date_symbols);
        EXPECT_EQ(10, date_count);
    }
    {
        const auto not_time_symbols = [](char c) {
            return !(c == '.' || std::isdigit(static_cast<unsigned char>(c)));
        };
        auto counted = clsc::count_until(date_time.cbegin() + date_count + 1, date_time.cend(),
                                         not_time_symbols);
        EXPECT_EQ(5, counted);
    }
}
