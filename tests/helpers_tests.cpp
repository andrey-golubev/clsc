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
