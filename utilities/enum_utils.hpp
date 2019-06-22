// Copyright 2018-2019 Andrey Golubev
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

#ifndef _ENUM_UTILS_HPP_
#define _ENUM_UTILS_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

namespace clsc {
#define VAARGS_LEN(...) (sizeof((char[]){__VA_ARGS__}) / sizeof(char))

#define MAKE_ENUM(name, ...)                                                   \
  enum name { __VA_ARGS__ };                                                   \
  const auto &values_of_##name() {                                             \
    static std::array<name, VAARGS_LEN(__VA_ARGS__)> values = {__VA_ARGS__};   \
    return values;                                                             \
  }                                                                            \
  bool belongs_to_##name(uint64_t value) {                                     \
    const auto &values = values_of_##name();                                   \
    return std::find(values.cbegin(), values.cend(), value) != values.cend();  \
  }
} // namespace clsc

#endif // _ENUM_UTILS_HPP_
