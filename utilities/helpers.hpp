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

#ifndef _HELPERS_HPP_
#define _HELPERS_HPP_

#include <algorithm>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace clsc {
namespace helpers {
template<typename CharT, typename Traits = std::char_traits<CharT>>
std::vector<std::basic_string<CharT>> split(const std::basic_string<CharT>& src, CharT delimiter) {
    std::vector<std::basic_string<CharT>> dst;
    std::basic_istringstream<CharT> ss_src(src);
    std::basic_string<CharT> tmp;
    while (std::getline(ss_src, tmp, delimiter)) {
        dst.push_back(tmp);
    }
    return dst;
}

struct trim_both_sides_tag {};
template<typename CharT, typename Traits = std::char_traits<CharT>>
void trim(trim_both_sides_tag, std::basic_string<CharT>& src) {
    const auto is_ignored = [](CharT x) { return std::isspace(int(x)); };

    auto back = std::find_if_not(src.rbegin(), src.rend(), is_ignored);
    src.erase(back.base(), src.end());

    auto front = std::find_if_not(src.begin(), src.end(), is_ignored);
    src.erase(src.begin(), front);
}

template<typename CharT, typename Iterator, typename UnaryOp,
         typename Traits = std::char_traits<CharT>,
         // Note: enable_if is necessary to disambiguate UnaryOp from CharT
         typename = typename std::enable_if<std::is_invocable<
             UnaryOp, typename std::iterator_traits<Iterator>::value_type>::value>::type>
std::basic_string<CharT> join(Iterator first, Iterator last, UnaryOp op, CharT sep = CharT(',')) {
    if (first == last) {
        return {};
    }

    std::basic_string<CharT> accumulator = op(*first);
    ++first;
    while (first != last) {
        accumulator += sep;
        accumulator += op(*first);
        ++first;
    }
    return accumulator;
}

template<typename CharT, typename Iterator, typename Traits = std::char_traits<CharT>>
std::basic_string<CharT> join(Iterator first, Iterator last, CharT sep = CharT(',')) {
    return join<CharT>(
        first, last,
        [](auto x) {
            using iterator_value_type = std::decay_t<decltype(x)>;
            if constexpr (std::is_same<iterator_value_type, std::decay_t<CharT>>::value) {
                return std::basic_string<CharT>(1, x);
            } else {
                // explicit cast to std::basic_string<> type - assume user type
                // to have the corresponding operator
                return std::basic_string<CharT>(x);
            }
        },
        sep);
}

template<typename Callable> class simple_scope_guard {
    Callable m_f;

public:
    [[nodiscard]] simple_scope_guard(Callable f) : m_f(f) {}
    ~simple_scope_guard() { m_f(); }
};

template<typename Callable> [[nodiscard]] decltype(auto) make_simple_scope_guard(Callable f) {
    return simple_scope_guard<Callable>{f};
}

}  // namespace helpers
}  // namespace clsc

#endif  // _HELPERS_HPP_
