#include <memory_concepts.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(memory_concepts_tests, fallback_allocator)
{
    clsc::fallback_allocator<clsc::malloc_allocator, clsc::null_allocator> allocator;
    int* my_ptr = (int*)allocator.allocate(1);
    allocator.deallocate(my_ptr);
    std::cout << my_ptr;
}
