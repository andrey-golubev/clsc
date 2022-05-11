// Copyright 2022 Andrey Golubev
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

#include <callable_traits.hpp>
#include <gtest/gtest.h>

TEST(callable_traits_tests, callable_traits) {
    auto captureless_lambda = [] (int x) { return x; };
    auto capturefull_lambda = [&] (int x, int y) { return x + captureless_lambda(y); };
    auto captureless_auto_lambda = [] (auto x) { return x; };
    auto capturefull_auto_lambda = [&] (auto x, int y) { return x + captureless_lambda(y); };

    EXPECT_EQ(1, clsc::callable_traits (captureless_lambda)::args_size);
    EXPECT_EQ(2, clsc::callable_traits (capturefull_lambda)::args_size);
    EXPECT_EQ(1, clsc::callable_traits (captureless_auto_lambda)::args_size);
    EXPECT_EQ(2, clsc::callable_traits (capturefull_auto_lambda)::args_size);
}
