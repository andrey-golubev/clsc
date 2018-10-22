#include <clear.hpp>
#include <gtest/gtest.h>

#include <cstring>
#include <list>
#include <memory>

TEST(clear_tests, scalar_type)
{
    int val = 1;
    clsc::clear(val);
    ASSERT_EQ(0, val);
    clsc::clear<int>(val, [](auto& val) { val = 1; });
    ASSERT_EQ(1, val);
}

TEST(clear_tests, array_type)
{
    int val[] = { 1, 2, 3, 4, 5 };
    int zeros[] = { 0, 0, 0, 0, 0 };
    clsc::clear(val);
    ASSERT_EQ(*zeros, *val) << "wrong default clear for array";
    int same[] = { 1, 2, 3, 4, 5 };
    clsc::clear<int[sizeof(val)/sizeof(val[0])]>(val, [](auto& o) { o[0] = 1; o[1] = 2; o[2] = 3; o[3] = 4; o[4] = 5; });
    ASSERT_EQ(*same, *val) << "wrong custom clear for array";
}

TEST(clear_tests, pod_object_type)
{
    struct s { int val; } st; st.val = 1;
    clsc::clear(st);
    ASSERT_EQ(0, st.val);
    clsc::clear<s>(st, [](auto& st) { st.val = 1; });
    ASSERT_EQ(1, st.val);
}

TEST(clear_tests, non_pod_object_type)
{
    struct s { int val = 1; } st;
    clsc::clear<s>(st, [](auto& st) { st.val = 0; });
    ASSERT_EQ(0, st.val);
    clsc::clear<s>(st, [](auto& st) { st.val = 1; });
    ASSERT_EQ(1, st.val);
}

TEST(clear_tests, ptr_type)
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
