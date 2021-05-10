// Copyright 2020 Andrey Golubev
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

#include <iterator>
#include <utility>

/**
 * \file count_until.hpp
 * \brief File defines count_until algorithms.
 */
namespace clsc {
template<typename InputIt, typename UnaryPredicate>
std::pair<InputIt, typename std::iterator_traits<InputIt>::difference_type>
count_until(InputIt first, InputIt last, UnaryPredicate p) {
    typename std::iterator_traits<InputIt>::difference_type ret = 0;
    for (; first != last; ++first, ++ret) {
        if (p(*first)) {
            break;
        }
    }
    return {first, ret};
}

template<typename InputIt, typename N, typename UnaryPredicate>
std::pair<InputIt, typename std::iterator_traits<InputIt>::difference_type>
count_until_n(InputIt first, N n, UnaryPredicate p) {
    typename std::iterator_traits<InputIt>::difference_type ret = 0;
    for (; n--; ++first, ++ret) {
        if (p(*first)) {
            break;
        }
    }
    return {first, ret};
}
}  // namespace clsc
