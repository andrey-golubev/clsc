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

#include <applications.hpp>

#include "common.hpp"

#include <gtest/gtest.h>

TEST(matrix, equality) {
    clsc::applications::Matrix<int> expected({{1, 2}, {3, 4}});
    auto actual = expected;
    EXPECT_EQ(expected, actual);
    actual.data()[0][0] = 4;
    EXPECT_NE(expected, actual);
}

TEST(matrix, multiply_accumulate_square_matrix) {
    clsc::applications::multiply_accumulate_matrix<int, std::plus<int>, std::multiplies<int>> op{};

    clsc::applications::Matrix<int> x({{1, 2}, {3, 4}});
    clsc::applications::Matrix<int> y({{5, 6}, {7, 8}});

    EXPECT_EQ(2, x.data().size());
    EXPECT_EQ(2, x.data()[0].size());
    EXPECT_EQ(x.data().size(), y.data().size());
    EXPECT_EQ(x.data()[0].size(), y.data()[0].size());

    auto z = op(x, y);
    EXPECT_EQ(2, z.data().size());
    EXPECT_EQ(2, z.data()[0].size());
    EXPECT_EQ(clsc::applications::Matrix<int>({{19, 22}, {43, 50}}), z);
}

TEST(matrix, multiply_accumulate_non_square_matrix) {
    clsc::applications::multiply_accumulate_matrix<int, std::plus<int>, std::multiplies<int>> op{};

    clsc::applications::Matrix<int> x({{1, 2}, {3, 4}, {5, 6}});
    clsc::applications::Matrix<int> y({{7}, {8}});

    EXPECT_EQ(3, x.data().size());
    EXPECT_EQ(2, x.data()[0].size());
    EXPECT_EQ(2, y.data().size());
    EXPECT_EQ(1, y.data()[0].size());

    auto z = op(x, y);
    EXPECT_EQ(3, z.data().size());
    EXPECT_EQ(1, z.data()[0].size());
    EXPECT_EQ(clsc::applications::Matrix<int>({{23}, {53}, {83}}), z);
}

TEST(friends, noop) {
    clsc::applications::Matrix<bool> friend_matrix(
        {{true, false, true}, {false, true, false}, {true, false, true}});

    auto same_matrix = clsc::applications::find_friends_n(friend_matrix, 0);
    EXPECT_EQ(friend_matrix, same_matrix);
}

TEST(friends, indirect_friends) {
    // Matrix of the form:
    /*
          A  B  C  D
        A 1  0  1  0
        B 0  1  1  0
        C 1  1  1  0
        D 0  0  0  1
    */
    clsc::applications::Matrix<bool> friend_matrix({{true, false, true, false},
                                                    {false, true, true, false},
                                                    {true, true, true, false},
                                                    {false, false, false, true}});
    // An (extra) indirect friend of A should be B. D is not a friend of any other
    clsc::applications::Matrix<bool> expected({{true, true, true, false},
                                               {true, true, true, false},
                                               {true, true, true, false},
                                               {false, false, false, true}});

    auto actual = clsc::applications::find_friends_n(friend_matrix, 1);
    EXPECT_EQ(expected, actual);
}
