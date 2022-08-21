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

#include "besc_lexer.hpp"
#include "source_location.hpp"
#include "tokens.hpp"

#include <helpers.hpp>

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace clsc {
namespace bes {

namespace {
std::unordered_map<std::string_view, token>& token_registry() {
    static std::unordered_map<std::string_view, token> registry;
    return registry;
}

inline token register_token(token::value value, const char* pattern) {
    token t{value};
    token_registry().emplace(pattern, value);
    return t;
}

#define CLSC_BES_NEW_TOKEN(NAME, pattern)                                                          \
    const ::clsc::bes::token TOKEN_##NAME = register_token(token::value::NAME, pattern);

CLSC_BES_NEW_TOKEN(OR, "||");
CLSC_BES_NEW_TOKEN(AND, "&&");
CLSC_BES_NEW_TOKEN(NOT, "~");
CLSC_BES_NEW_TOKEN(XOR, "^");
CLSC_BES_NEW_TOKEN(ARROW_RIGHT, "->");
CLSC_BES_NEW_TOKEN(ARROW_LEFT, "<-");
CLSC_BES_NEW_TOKEN(EQ, "==");
CLSC_BES_NEW_TOKEN(NEQ, "!=");

CLSC_BES_NEW_TOKEN(ASSIGN, "=");
CLSC_BES_NEW_TOKEN(ALIAS, "symbol");  // TODO: rename to "alias"?
CLSC_BES_NEW_TOKEN(VAR, "var");
CLSC_BES_NEW_TOKEN(EVAL, "eval");

CLSC_BES_NEW_TOKEN(SEMICOLON, ";");

inline int spaces_for_tab(char tab) {
    switch (tab) {
    case '\t':
        return 4;
    case ' ':
        return 1;
    default:
        assert(false && "unreachable");
    }
    return 0;
}

class lexer_state {
    std::string m_buffer{};
    source_location m_loc{};

public:
    lexer_state() { m_buffer.reserve(50); }

    annotated_token read_token(source_location new_loc) {
        auto guard = helpers::make_simple_scope_guard([this, &new_loc]() {
            m_buffer.clear();
            m_loc = new_loc;
        });
        const auto& registry = token_registry();
        auto it = registry.find(std::string_view(m_buffer));
        if (it == registry.end()) {
            // TODO: do basic identifier checks? (e.g. "starts with letter or
            // underscore", etc.)
            return {token{token::_BESID}, m_loc};
        }
        return {it->second, m_loc};
    }

    void add(char c) { m_buffer.push_back(c); }
    bool empty() const { return m_buffer.empty(); }

    void trim() { clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, m_buffer); }

    const std::string& buffer() const { return m_buffer; }
};

void read_token(std::iostream& out, lexer_state& state, source_location new_loc) {
    state.trim();
    if (!state.empty()) {
        out << std::string(state.read_token(new_loc));
        out << '\n';
    }
}

}  // namespace

void lexer::tokenize() {
    lexer_state state{};

    int line = 0;
    int column = 0;

    for (char current = '\0'; m_in.get(current).good();) {
        switch (current) {
        case '\0':
            assert(false && "should be rejected by the loop");
        case '\n': {
            ++line;
            column = 0;
            [[fallthrough]];
        }
        case '\r': {
            column = 0;
            continue;
        }
        case ';': {
            read_token(m_out, state, {line, column});
            state.add(current);
            ++column;
            read_token(m_out, state, {line, column});
            break;
        }
        case '\t':
        case ' ': {
            column += spaces_for_tab(current);
            read_token(m_out, state, {line, column});
            continue;
        }
        default: {
            state.add(current);
            break;
        }
        }
        ++column;
    }

    // in case there's anything left in the buffer
    read_token(m_out, state, {line, column});
}

}  // namespace bes
}  // namespace clsc
