#include <ring_array.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

TEST(ring_array_test, init)
{
    clsc::ring_array<int, ARRAY_SIZE> buf;
}

TEST(ring_array_test, push_back)
{
    clsc::ring_array<int, ARRAY_SIZE> buf;
    for (int i = 0; i < ARRAY_SIZE; i++)
        buf.push_back(i);
}

TEST_F(ring_array_tests, front)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(i, m_buf.move_front());
}

TEST_F(ring_array_tests, back)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(ARRAY_SIZE-(i+1), m_buf.move_back());
}

TEST_F(ring_array_tests, random_access)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(i, m_buf[i]);
}
