// Copyright 2018 Andrey Golubev
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

#include <helpers.hpp>

#include <gtest/gtest.h>

TEST(helpers_tests, split_string) {
    std::string str("hello world");
    const auto array = clsc::helpers::split(str, ' ');
    EXPECT_EQ("hello", array[0]);
    EXPECT_EQ("world", array[1]);
}

TEST(helpers_tests, split_wstring) {
    std::wstring str(L"hello world");
    const auto array = clsc::helpers::split(str, L' ');
    EXPECT_EQ(L"hello", array[0]);
    EXPECT_EQ(L"world", array[1]);
}

TEST(helpers_tests, split_u16string) {
    std::u16string str(u"hello world");
    const auto array = clsc::helpers::split(str, u' ');
    EXPECT_EQ(u"hello", array[0]);
    EXPECT_EQ(u"world", array[1]);
}

TEST(helpers_tests, split_u32string) {
    std::u32string str(U"hello world");
    const auto array = clsc::helpers::split(str, U' ');
    EXPECT_EQ(U"hello", array[0]);
    EXPECT_EQ(U"world", array[1]);
}

TEST(helpers_tests, trim_string_empty) {
    std::string str;
    clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, str);
    EXPECT_TRUE(str.empty());
}

TEST(helpers_tests, trim_string_nothing) {
    std::wstring str(L"nothing to trim");
    clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, str);
    EXPECT_EQ(L"nothing to trim", str);
}

TEST(helpers_tests, trim_string_left) {
    std::string str(" h e l l o");
    clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, str);
    EXPECT_EQ("h e l l o", str);
}

TEST(helpers_tests, trim_string_right) {
    std::string str("h e l l o ");
    clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, str);
    EXPECT_EQ("h e l l o", str);
}

TEST(helpers_tests, trim_string_both_sides) {
    std::u16string str(u" h e l l o ");
    clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, str);
    EXPECT_EQ(u"h e l l o", str);
}

TEST(helpers_tests, trim_string_more_than_one) {
    std::u32string str(U"  hello, world!    ");
    clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, str);
    EXPECT_EQ(U"hello, world!", str);
}

TEST(helpers_tests, join_empty_range) {
    std::vector<int> empty{};
    auto actual = clsc::helpers::join<char>(empty.begin(), empty.end(), [](int x) {
        return std::to_string(x);
    });
    ASSERT_EQ(actual, std::string{});
}

TEST(helpers_tests, join_nonempty_range) {
    std::vector<int> nonempty{1, 2, -11, 235};
    auto actual = clsc::helpers::join<char>(nonempty.begin(), nonempty.end(), [](int x) {
        return std::to_string(x);
    });
    ASSERT_EQ(actual, "1,2,-11,235");
}

TEST(helpers_tests, join_single_element) {
    std::vector<wchar_t> nonempty{L'a'};
    auto actual = clsc::helpers::join<wchar_t>(nonempty.begin(), nonempty.end());
    ASSERT_EQ(actual, L"a");
}

TEST(helpers_tests, join_custom_separator) {
    std::vector<char32_t> nonempty{U'a', U'b', U'c'};
    auto actual = clsc::helpers::join<char32_t>(nonempty.begin(), nonempty.end(), ' ');
    ASSERT_EQ(actual, U"a b c");
}
