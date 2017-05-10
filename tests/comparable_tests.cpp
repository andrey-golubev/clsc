#include <comparable.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "common.hpp"

TEST(comparable_tests, simple_inheritance)
{
    class A : public clsc::comparable<A>
    {
        int m_val;
    public:
        A() = delete;
        A(const int& val) : m_val(val) {}

        int operator()(const A& rhs)
        {
            return m_val - rhs.m_val;
        }
    };

    A a(1), b(2);
    tests_common::compare(a, b);
}

TEST(comparable_tests, multiple_inheritance)
{
    class operator_mock
    {
        int m_val;
    public:
        operator_mock() = delete;
        operator_mock(int val) : m_val(val) {}
        int operator()(const operator_mock& rhs)
        {
            auto r = m_val - rhs.m_val;
            return m_val - rhs.m_val;
        }
    };

    class A : public clsc::comparable<A>, public operator_mock
    {
    public:
        A() = delete;
        A(int val) : operator_mock(val) {}
    };

    A a(1), b(2);
    tests_common::compare(a, b);
}

TEST(comparable_tests, multilevel_inheritance)
{
    class operator_mock : public clsc::comparable<operator_mock>
    {
        int m_val;
    public:
        operator_mock() = delete;
        operator_mock(int val) : m_val(val) {}
        int operator()(const operator_mock& rhs)
        {
            return m_val - rhs.m_val;
        }
    };

    class A : public operator_mock
    {
    public:
        A() = delete;
        A(int val) : operator_mock(val) {}
    };

    A a(1), b(2);
    tests_common::compare(a, b);
}

TEST(adjustable_comparable_tests, operator_exists)
{
    class A : public clsc::adjustable_comparable<A>
    {
        int m_val;
    public:
        A() = delete;
        A(const int& val) : m_val(val) {}

        int operator()(const A& rhs)
        {
            return m_val - rhs.m_val;
        }
    };

    A a(1), b(2);
    tests_common::compare(a, b);
}

TEST(adjustable_comparable_tests, no_operator)
{
    class A : public clsc::adjustable_comparable<A>
    {
        int m_val;
    public:
        A() = delete;
        A(const int& val) : m_val(val) {}
    };
    A a(1), b(2);
}
