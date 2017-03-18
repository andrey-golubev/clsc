#include <comparable.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

template<typename T>
void compare(T&& a, T&& b)
{
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);

    ASSERT_FALSE(a > b);
    ASSERT_TRUE(a < b);

    ASSERT_FALSE(a >= b);
    ASSERT_TRUE(a <= b);
}

TEST(comparable_tests, single_inheritance)
{
    class A : public clsc::comparable<A>
    {
        int m_val;
    public:
        A() : m_val(0) {}
        A(const int& val) : m_val(val) {}

        bool operator<(const A& rhs)
        {
            return m_val < rhs.m_val;
        }

        bool operator==(const A& rhs)
        {
            return m_val == rhs.m_val;
        }
    };

    A a(1), b(2);
    compare(a, b);
}

TEST(comparable_tests, multiple_inheritance)
{
    class operator_mock
    {
    public:
        bool operator<(const operator_mock&)
        {
            return true;
        }

        bool operator==(const operator_mock&)
        {
            return false;
        }
    };

    class A : public clsc::comparable<A>, public operator_mock
    {
    };

    A a, b;
    compare(a, b);
}

TEST(comparable_tests, multiple_level_inheritance)
{
    class operator_mock : public clsc::comparable<operator_mock>
    {
    public:
        bool operator<(const operator_mock&)
        {
            return true;
        }

        bool operator==(const operator_mock&)
        {
            return false;
        }
    };

    class A : public operator_mock
    {
    };

    A a, b;
    compare(a, b);
}

TEST(adjustable_comparable_tests, both_operators_defined)
{
    class A : public clsc::adjustable_comparable<A>
    {
        int m_val;
    public:
        A() : m_val(0) {}
        A(const int& val) : m_val(val) {}

        bool operator<(const A& rhs)
        {
            return m_val < rhs.m_val;
        }

        bool operator==(const A& rhs)
        {
            return m_val == rhs.m_val;
        }
    };

    A a(1), b(2);
    compare(a, b);
}

TEST(adjustable_comparable_tests, equal_to_operator_defined)
{
    class A : public clsc::adjustable_comparable<A>
    {
        int m_val;
    public:
        A() : m_val(0) {}
        A(const int& val) : m_val(val) {}

        bool operator==(const A& rhs)
        {
            return m_val == rhs.m_val;
        }
    };

    A a(1), b(2);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
}

TEST(adjustable_comparable_tests, less_than_operator_defined)
{
    class A : public clsc::adjustable_comparable<A>
    {
        int m_val;
    public:
        A() : m_val(0) {}
        A(const int& val) : m_val(val) {}

        bool operator<(const A& rhs)
        {
            return m_val < rhs.m_val;
        }
    };

    A a(1), b(2);
    ASSERT_TRUE(a < b);
    ASSERT_FALSE(a >= b);
}
