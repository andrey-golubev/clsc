#include <memory_concepts.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace
{
    constexpr std::size_t PLAIN_SIZE = 1;

    template<typename T>
    struct SIZE_ON_TYPE
    {
        constexpr static std::size_t value = PLAIN_SIZE * sizeof(T);
    };

    template<typename T>
    auto SIZE = SIZE_ON_TYPE<T>::value;
}

TEST(memory_concepts_tests, fallback_allocator)
{
    clsc::fallback_allocator<clsc::malloc_allocator, clsc::null_allocator> allocator;
    auto my_ptr = allocator.allocate(SIZE<int>);
    EXPECT_NE(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, SIZE<int>);
    allocator.deallocate(my_ptr);
    EXPECT_EQ(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, 0);
}

TEST(memory_concepts_tests, default_allocator)
{
    clsc::default_allocator allocator;
    auto my_ptr = allocator.allocate(SIZE<int>);
    EXPECT_NE(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, SIZE<int>);
    allocator.deallocate(my_ptr);
    EXPECT_EQ(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, 0);
}

TEST(memory_concepts_tests, simple_allocator)
{
    clsc::simple_allocator allocator;
    auto my_ptr = allocator.allocate(SIZE<int>);
    EXPECT_NE(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, SIZE<int>);
    allocator.deallocate(my_ptr);
    EXPECT_EQ(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, 0);
}
