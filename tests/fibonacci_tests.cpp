// Copyright 2021 Andrey Golubev
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

#include <fibonacci.hpp>

#include <gtest/gtest.h>

#include <numeric>

TEST(fibonacci_tests, zero) {
    const std::uint64_t expected = 0;
    EXPECT_EQ(expected, clsc::fibonacci(0));
}

TEST(fibonacci_tests, one) {
    const std::uint64_t expected = 1;
    EXPECT_EQ(expected, clsc::fibonacci(1));
}

TEST(fibonacci_tests, two) {
    const std::uint64_t expected = 1;
    EXPECT_EQ(expected, clsc::fibonacci(2));
}

TEST(fibonacci_tests, three) {
    const std::uint64_t expected = 2;
    EXPECT_EQ(expected, clsc::fibonacci(3));
}

TEST(fibonacci_tests, four) {
    const std::uint64_t expected = 3;
    EXPECT_EQ(expected, clsc::fibonacci(4));
}

TEST(fibonacci_tests, five) {
    const std::uint64_t expected = 5;
    EXPECT_EQ(expected, clsc::fibonacci(5));
}

TEST(fibonacci_tests, random_inputs) {
    int array_of_n[65] = {};
    std::iota(std::begin(array_of_n), std::end(array_of_n), 6);

    for (int n : array_of_n) {
        const std::uint64_t expected = clsc::fibonacci(n - 1) + clsc::fibonacci(n - 2);
        EXPECT_EQ(expected, clsc::fibonacci(n));
    }
}
