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

#include <cstddef>
#include <iterator>
#include <ostream>
#include <string>

namespace clsc {
namespace bes {
struct source_location {
    int line = 0;
    int column = 0;

    // technical fields
    std::size_t offset = 0;
    std::size_t length = 0;

    source_location(int l, int c, std::size_t o) : line(l), column(c), offset(o) {}
    source_location(int l, int c, std::size_t o, std::size_t len)
        : line(l), column(c), offset(o), length(len) {}

    source_location() = default;
    source_location(const source_location&) = default;
    source_location(source_location&&) = default;
    source_location& operator=(const source_location&) = default;
    source_location& operator=(source_location&&) = default;
    ~source_location() = default;

    explicit operator std::string() const {
        return std::to_string(line) + ':' + std::to_string(column);
    }

    friend std::ostream& operator<<(std::ostream& os, const source_location& x) {
        return os << x.line << ':' << x.column;
    }
};
}  // namespace bes
}  // namespace clsc
