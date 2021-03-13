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

#include "group_theory_bits.hpp"

#include <cassert>

/**
 * \file algorithm.hpp
 * \brief File defines a number of algorithms. Currently a file for "everything".
 */
namespace clsc {

namespace detail {
template<typename Regular, typename Integer, typename SemigroupOperation>
Regular power_accumulate_semigroup(Regular r, Regular a, Integer n, SemigroupOperation op) {
    // requires that domain of SemigroupOperation is Regular type
    while (true) {
        using clsc::detail::half;
        using clsc::detail::odd;
        if (odd(n)) {
            r = op(r, a);
            if (n == Integer(1)) {
                return r;
            }
        }
        n = half(n);
        a = op(a, a);
    }
}
}  // namespace detail

template<typename Regular, typename Integer, typename SemigroupOperation>
Regular power_semigroup(Regular a, Integer n, SemigroupOperation op) {
    assert(n > 0);
    using clsc::detail::half;
    using clsc::detail::odd;
    while (!odd(n)) {
        n = half(n);
        a = op(a, a);
    }
    if (n == Integer(1)) {
        return a;
    }
    return detail::power_accumulate_semigroup(a, op(a, a), half(n - 1), op);
}

template<typename Regular, typename Integer, typename MonoidOperation>
Regular power_monoid(Regular a, Integer n, MonoidOperation op) {
    assert(n >= 0);
    if (n == Integer(0)) {
        using clsc::detail::identity_element;
        return identity_element(op);
    }
    return power_semigroup(a, n, op);
}

template<typename Regular, typename Integer, typename GroupOperation>
Regular power_group(Regular a, Integer n, GroupOperation op) {
    // n - any value
    if (n < 0) {
        n = -n;
        using clsc::detail::inverse_element;
        a = inverse_element(op)(a);
    }
    return power_monoid(a, n, op);
}

}  // namespace clsc
