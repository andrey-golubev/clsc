#ifndef _COMMON_HPP_
#define _COMMON_HPP_

namespace tests_common
{
    template<typename T>
    void compare(T&& a, T&& b)
    {
        EXPECT_FALSE(a == b);
        EXPECT_TRUE(a != b);

        EXPECT_TRUE(a < b);
        EXPECT_TRUE(a <= b);

        EXPECT_FALSE(a > b);
        EXPECT_FALSE(a >= b);
    }
}

#endif // _COMMON_HPP_
