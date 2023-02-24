// Copyright 2023 Andrey Golubev
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

#include <type_algorithm.hpp>

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

struct MyType {};
void foofunc() {}

TEST(type_algorithm_tests, any_of) {
    static_assert(clsc::any_of<std::is_void, int, double, void, MyType>::value);
    static_assert(clsc::any_of<std::is_integral, int>::value);
    static_assert(!clsc::any_of<std::is_function, int, MyType>::value);
    static_assert(!clsc::any_of<std::is_fundamental, MyType, std::string>::value);
}

TEST(type_algorithm_tests, all_of) {
    static_assert(clsc::all_of<std::is_floating_point, long double>::value);
    static_assert(
        clsc::all_of<std::is_reference, const int&, std::string&, volatile const MyType&>::value);
    static_assert(!clsc::all_of<std::is_integral, int, MyType, double, float>::value);
    static_assert(!clsc::all_of<std::is_void, void, int*>::value);
}

TEST(type_algorithm_tests, none_of) {
    static_assert(clsc::none_of<std::is_floating_point, int, char, unsigned long long>::value);
    static_assert(clsc::none_of<std::is_void, MyType>::value);
    static_assert(!clsc::none_of<std::is_function, int, decltype(foofunc)>::value);
    static_assert(!clsc::none_of<std::is_fundamental, decltype(nullptr)>::value);
}
