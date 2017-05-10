#include <ring_array.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "common.hpp"

namespace
{
    constexpr int ARRAY_SIZE = 5;

    class ring_array_tests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            for (int i = 0; i < ARRAY_SIZE; i++)
                m_buf.push_back(i);
        }

        clsc::ring_array<int, ARRAY_SIZE> m_buf;
    };
}

TEST(ring_array_test, can_compare)
{
    clsc::ring_array<int, 1> a, b;
    a.push_back(1); b.push_back(2);
    tests_common::compare(a, b);
}

TEST(ring_array_test, init)
{
    clsc::ring_array<int, ARRAY_SIZE> buf;
}

TEST(ring_array_test, push_front)
{
    clsc::ring_array<int, ARRAY_SIZE> buf;
    for (int i = 0; i < ARRAY_SIZE; i++)
        buf.push_front(i);

    // TODO: is this a bug?
    EXPECT_EQ(0, buf.at(0)); // push_front on first element has specific behaviour
    for (int i = 1; i < ARRAY_SIZE; i++)
        EXPECT_EQ(ARRAY_SIZE - i, buf.at(i));
}

TEST(ring_array_test, push_back)
{
    clsc::ring_array<int, ARRAY_SIZE> buf;
    for (int i = 0; i < ARRAY_SIZE; i++)
        buf.push_back(i);
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(i, buf.at(i));
}

TEST_F(ring_array_tests, front)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(0, m_buf.front());
}

TEST_F(ring_array_tests, move_front)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(i, m_buf.move_front());
}

TEST_F(ring_array_tests, back)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(ARRAY_SIZE-1, m_buf.back());
}

TEST_F(ring_array_tests, move_back)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(ARRAY_SIZE-(i+1), m_buf.move_back());
}

TEST_F(ring_array_tests, random_access)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(i, m_buf[i]);

    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(i, m_buf.at(i));
}
