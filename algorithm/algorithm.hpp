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

#pragma once

#include "group_theory_bits.hpp"

#include <algorithm>
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

namespace experimental {
template<typename InputIt, typename OutputIt>
void median(InputIt first1, InputIt last1, InputIt first2, InputIt last2, OutputIt d_first) {
    // requires [first1, last1) and [first2, last2) are sorted ranges
    auto d1 = std::distance(first1, last1);
    auto d2 = std::distance(first2, last2);
    using distance_type = decltype(d1);
    if (d1 == distance_type(0)) {
        if (d2 == distance_type(0))
            return;

        std::advance(first2, (d2 - 1) / 2);
        *d_first = *first2;
        if (d2 % 2 == distance_type(0)) {
            ++d_first;
            ++first2;
            *d_first = *first2;
        }
        return;
    }
    if (d2 == distance_type(0)) {
        if (d1 == distance_type(0))
            return;

        std::advance(first1, (d1 - 1) / 2);
        *d_first = *first1;
        if (d1 % 2 == distance_type(0)) {
            ++d_first;
            ++first1;
            *d_first = *first1;
        }
        return;
    }
    if (d1 < 3 && d2 < 3) {
        InputIt subrange[4] = {};
        {
            auto d_subrange = &subrange[0];
            for (auto f = first1; f != last1; ++f, ++d_subrange) {
                *d_subrange = f;
            }
            for (auto f = first2; f != last2; ++f, ++d_subrange) {
                *d_subrange = f;
            }
        }
        std::sort(std::begin(subrange), std::end(subrange), [](auto x, auto y) { return *x < *y; });

        auto midpoint = &subrange[0];
        std::advance(midpoint, (d1 + d2 - 1) / 2);
        *d_first = **midpoint;
        if ((d1 + d2) % 2 == distance_type(0)) {
            ++d_first;
            ++midpoint;
            *d_first = **midpoint;
        }
        return;
    }

    if (!(*first1 < *first2)) {
        using std::swap;
        swap(first1, first2);
        swap(last1, last2);
        swap(d1, d2);
    }

    auto midpoint1 = first1;
    std::advance(midpoint1, (d1 - 1) / 2);
    auto midpoint2 = first2;
    std::advance(midpoint2, (d2 - 1) / 2);

    if (*midpoint1 < *midpoint2) {
        median(midpoint1, last1, first2, midpoint2, d_first);
    } else {
        median(first1, midpoint1, midpoint2, last2, d_first);
    }
}

}  // namespace experimental

}  // namespace clsc
