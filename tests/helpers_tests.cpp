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

TEST(helpers_tests, split_string)
{
    std::string str("hello world");
    const auto array = clsc::helpers::split(str, ' ');
    EXPECT_EQ("hello", array[0]);
    EXPECT_EQ("world", array[1]);
}

TEST(helpers_tests, split_wstring)
{
    std::wstring str(L"hello world");
    const auto array = clsc::helpers::split(str, L' ');
    EXPECT_EQ(L"hello", array[0]);
    EXPECT_EQ(L"world", array[1]);
}

TEST(helpers_tests, split_u16string)
{
    std::u16string str(u"hello world");
    const auto array = clsc::helpers::split(str, u' ');
    EXPECT_EQ(u"hello", array[0]);
    EXPECT_EQ(u"world", array[1]);
}

TEST(helpers_tests, split_u32string)
{
    std::u32string str(U"hello world");
    const auto array = clsc::helpers::split(str, U' ');
    EXPECT_EQ(U"hello", array[0]);
    EXPECT_EQ(U"world", array[1]);
}
