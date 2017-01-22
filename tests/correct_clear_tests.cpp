#include <correct_clear.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <list>
#include <memory>

TEST(correct_clear_tests, scalar_test)
{
    int val = 1;
    clsc::clear(val);
    ASSERT_EQ(0, val);
    clsc::clear<int>(val, [](auto& val) { val = 1; });
    ASSERT_EQ(1, val);
}

TEST(correct_clear_tests, array_test)
{
    int val[] = { 1, 2, 3, 4, 5 };
    clsc::clear(*val); // only first value changes
    ASSERT_EQ(0, *val);
    clsc::clear<int>(*val, [&val](int&) { std::memset(val, 0, sizeof(val)); });
    ASSERT_THAT(val, ::testing::ElementsAre(0, 0, 0, 0, 0));
    clsc::clear<int>(*val, [&val](int&) { val[0] = 1; val[1] = 2; val[2] = 3; val[3] = 4; val[4] = 5; });
    ASSERT_THAT(val, ::testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(correct_clear_tests, pod_object_test)
{
    struct s { int val; } st; st.val = 1;
    clsc::clear(st);
    ASSERT_EQ(0, st.val);
    clsc::clear<s>(st, [](auto& st) { st.val = 1; });
    ASSERT_EQ(1, st.val);
}

TEST(correct_clear_tests, non_pod_object_test)
{
    struct s { int val = 1; } st;
    clsc::clear<s>(st, [](auto& st) { st.val = 0; });
    ASSERT_EQ(0, st.val);
    clsc::clear<s>(st, [](auto& st) { st.val = 1; });
    ASSERT_EQ(1, st.val);
}

TEST(correct_clear_tests, ptr_test)
{
    {
        std::unique_ptr<int> val(new int); *val = 1;
        clsc::clear(*val);
        ASSERT_EQ(0, *val);
        clsc::clear<int>(*val, [](auto& val) { val = 1; });
        ASSERT_EQ(1, *val);
    }

    {
        std::shared_ptr<int> val(new int); *val = 1;
        clsc::clear(*val);
        ASSERT_EQ(0, *val);
        ASSERT_EQ(1, val.use_count()) << "wrong use_count() after clear";
        clsc::clear<int>(*val, [](auto& val) { val = 1; });
        ASSERT_EQ(1, *val);
        ASSERT_EQ(1, val.use_count()) << "wrong use_count() after clear";;
    }
}
