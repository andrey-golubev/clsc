#include <enum_beautifier.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

enum E
{
    A = 0,
    B = 1
};

TEST(enum_beautifier_tests, test1)
{
    clsc::enum_beautifier<E, E::A, E::B> e;
}
