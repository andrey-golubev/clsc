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

#pragma once

#include "algorithm.hpp"

#include <cstdint>
#include <functional>

/**
 * \file fibonacci.hpp
 * \brief File defines an algorithm for Fibonacci sequence generation. The algorithm is based on
 * linear algebra and has O(log N) complexity.
 */

namespace clsc {

namespace detail {

struct Matrix2x2 {
    std::uint64_t elements[2 * 2] = {};

    std::uint64_t e11() const { return elements[0]; }
    std::uint64_t e12() const { return elements[1]; }
    std::uint64_t e21() const { return elements[2]; }
    std::uint64_t e22() const { return elements[3]; }

    friend Matrix2x2 operator*(const Matrix2x2& a, const Matrix2x2& b) {
        const std::uint64_t e11 = a.e11() * b.e11() + a.e12() * b.e21();
        const std::uint64_t e12 = a.e11() * b.e12() + a.e12() * b.e22();
        const std::uint64_t e21 = a.e21() * b.e11() + a.e22() * b.e21();
        const std::uint64_t e22 = a.e21() * b.e12() + a.e22() * b.e22();
        return {e11, e12, e21, e22};
    }
};

struct Vector2 {
    std::uint64_t elements[2] = {0, 0};

    // square matrix multiplied by column vector.
    friend Vector2 operator*(const Matrix2x2& m, const Vector2& v) {
        const std::uint64_t e11 = m.e11() * v.elements[0] + m.e12() * v.elements[1];
        const std::uint64_t e21 = m.e21() * v.elements[0] + m.e22() * v.elements[1];
        return {e11, e21};
    }
};

Matrix2x2 identity_element(std::multiplies<Matrix2x2>) { return {1, 0, 0, 1}; }

}  // namespace detail

std::uint64_t fibonacci(std::uint32_t n) {
    assert(n >= 0);
    if (n == 0)
        return 0;
    const detail::Vector2 initial_value{1, 0};  // first two elements of fibonacci sequence
    const detail::Matrix2x2 fibonacci_matrix{1, 1, 1, 0};  // core matrix that defines the algorithm
    const detail::Vector2 result =
        power_monoid(fibonacci_matrix, n - 1, std::multiplies<detail::Matrix2x2>{}) * initial_value;
    return result.elements[0];
}

}  // namespace clsc
