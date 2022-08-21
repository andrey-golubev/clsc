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

#include "source_location.hpp"

#include <stdexcept>
#include <string>

#include <cctype>

namespace clsc {
namespace bes {
// each token needs:
// - name
// - enum value (and its string representation)
// - keyword (optional)
struct token {
    enum value {
        UNKNOWN,

        OR,
        AND,
        NOT,
        XOR,
        ARROW_RIGHT,
        ARROW_LEFT,
        EQ,
        NEQ,

        ASSIGN,
        ALIAS,
        VAR,
        EVAL,

        SEMICOLON,

        _BESID,  // not a keyword itself but special, needs care not to collide with user code
    };

    token::value id = value::UNKNOWN;

    constexpr token() = default;
    constexpr token(token::value value) : id(value) {}

    explicit operator std::string() const {
#define CASE(x)                                                                                    \
    case token::value::x:                                                                          \
        return #x;

        switch (this->id) {
            CASE(OR);
            CASE(AND);
            CASE(NOT);
            CASE(XOR);
            CASE(ARROW_RIGHT);
            CASE(ARROW_LEFT);
            CASE(EQ);
            CASE(NEQ);
            CASE(ASSIGN);
            CASE(ALIAS);
            CASE(VAR);
            CASE(EVAL);
            CASE(SEMICOLON);
            CASE(_BESID);
        default:
            throw std::runtime_error("Unknown token");
        }
        return {};

#undef CASE
    }
};

struct annotated_token {
    token tok;
    source_location loc;

    explicit operator std::string() const { return std::string(tok) + ' ' + std::string(loc); }
};

}  // namespace bes
}  // namespace clsc
