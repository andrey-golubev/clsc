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

#include <functional>
#include <stdexcept>
#include <string>

#include <cctype>
#include <type_traits>

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

        IDENTIFIER,  // not a keyword itself but special, needs care not to collide with user code

        LITERAL_TRUE,
        LITERAL_FALSE,
        LITERAL_STRING,
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
            CASE(IDENTIFIER);
            CASE(LITERAL_TRUE);
            CASE(LITERAL_FALSE);
            CASE(LITERAL_STRING);
        default:
            throw std::runtime_error("Unknown token");
        }
        return {};

#undef CASE
    }

    friend bool operator==(const token& x, const token& y) { return x.id == y.id; }
    friend bool operator!=(const token& x, const token& y) { return !(x.id == y.id); }
};

struct annotated_token {
    token tok;
    source_location loc;

    explicit operator std::string() const { return std::string(tok) + ' ' + std::string(loc); }
};

#define CLSC_BES_NEW_TOKEN_DECLARE(NAME) extern const ::clsc::bes::token TOKEN_##NAME;

#define CLSC_BES_FOR_EACH_GLOBAL_TOKEN(CALL)                                                       \
    CALL(OR)                                                                                       \
    CALL(AND)                                                                                      \
    CALL(NOT)                                                                                      \
    CALL(XOR)                                                                                      \
    CALL(ARROW_RIGHT)                                                                              \
    CALL(ARROW_LEFT)                                                                               \
    CALL(EQ)                                                                                       \
    CALL(NEQ)                                                                                      \
    CALL(ASSIGN)                                                                                   \
    CALL(ALIAS)                                                                                    \
    CALL(VAR)                                                                                      \
    CALL(EVAL)                                                                                     \
    CALL(SEMICOLON)                                                                                \
    CALL(LITERAL_TRUE)                                                                             \
    CALL(LITERAL_FALSE)

// forward declare all global tokens
CLSC_BES_FOR_EACH_GLOBAL_TOKEN(CLSC_BES_NEW_TOKEN_DECLARE)

#undef CLSC_BES_NEW_TOKEN_DECLARE

}  // namespace bes
}  // namespace clsc

template<> struct std::hash<clsc::bes::token> {
    std::size_t operator()(const clsc::bes::token& t) const noexcept {
        return std::hash<clsc::bes::token::value>{}(t.id);
    }
};
