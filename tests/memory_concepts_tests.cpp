// Copyright 2018 Andrey Golubev
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// ANDANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <memory_concepts.hpp>
#include <gtest/gtest.h>

TEST(memory_concepts_tests, fallback_allocator)
{
    clsc::fallback_allocator<clsc::malloc_allocator, clsc::null_allocator> allocator;
    auto my_ptr = allocator.allocate(sizeof(int));
    EXPECT_NE(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, sizeof(int));
    allocator.deallocate(my_ptr);
    EXPECT_EQ(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, 0);
}

TEST(memory_concepts_tests, stack_allocator)
{
    clsc::stack_allocator<1024> allocator;
    auto my_ptr = allocator.allocate(sizeof(int));
    EXPECT_NE(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, sizeof(int));
    allocator.deallocate(my_ptr);
    EXPECT_EQ(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, 0);
}

TEST(memory_concepts_tests, default_allocator)
{
    clsc::default_allocator allocator;
    auto my_ptr = allocator.allocate(sizeof(int));
    EXPECT_NE(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, sizeof(int));
    allocator.deallocate(my_ptr);
    EXPECT_EQ(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, 0);
}

TEST(memory_concepts_tests, simple_allocator)
{
    clsc::simple_allocator allocator;
    auto my_ptr = allocator.allocate(sizeof(int));
    EXPECT_NE(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, sizeof(int));
    allocator.deallocate(my_ptr);
    EXPECT_EQ(my_ptr.data, nullptr);
    EXPECT_EQ(my_ptr.size, 0);
}
