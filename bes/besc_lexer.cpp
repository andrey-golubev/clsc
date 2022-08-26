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
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace clsc {
namespace bes {

namespace {
std::unordered_map<std::string_view, token>& const_token_registry() {
    static std::unordered_map<std::string_view, token> registry;
    return registry;
}

inline token register_token(token::value value, const char* pattern) {
    token t{value};
    const_token_registry().emplace(pattern, value);
    return t;
}
}  // namespace

// define tokens
#define NEW_TOKEN(NAME, pattern)                                                                   \
    const ::clsc::bes::token TOKEN_##NAME = register_token(token::value::NAME, pattern);

#define NEW_NON_CONST_TOKEN(NAME) const ::clsc::bes::token TOKEN_##NAME = token{token::value::NAME};

// const tokens
NEW_TOKEN(OR, "||");
NEW_TOKEN(AND, "&&");
NEW_TOKEN(NOT, "~");
NEW_TOKEN(XOR, "^");
NEW_TOKEN(ARROW_RIGHT, "->");
NEW_TOKEN(ARROW_LEFT, "<-");
NEW_TOKEN(EQ, "==");
NEW_TOKEN(NEQ, "!=");
NEW_TOKEN(ASSIGN, "=");
NEW_TOKEN(ALIAS, "symbol");  // TODO: rename to "alias"?
NEW_TOKEN(VAR, "var");
NEW_TOKEN(EVAL, "eval");
NEW_TOKEN(SEMICOLON, ";");
NEW_TOKEN(LITERAL_TRUE, "true");
NEW_TOKEN(LITERAL_FALSE, "false");
NEW_TOKEN(PAREN_LEFT, "(");
NEW_TOKEN(PAREN_RIGHT, ")");

// non-const tokens
NEW_NON_CONST_TOKEN(UNKNOWN)
NEW_NON_CONST_TOKEN(IDENTIFIER)
NEW_NON_CONST_TOKEN(LITERAL_STRING)

#undef NEW_TOKEN
#undef NEW_NON_CONST_TOKEN

class lexer_state {
    std::string m_buffer{};
    source_location m_loc{};

    bool holds_valid_identifier_token() const {
        if (m_buffer.empty()) {
            return false;
        }

        const char extra_unacceptable_first_characters[] = {':'};
        if (std::isdigit(m_buffer[0]) ||
            std::any_of(std::begin(extra_unacceptable_first_characters),
                        std::end(extra_unacceptable_first_characters),
                        [&](char x) { return m_buffer[0] == x; })) {
            return false;
        }

        const auto is_acceptable_character = [](char x) {
            if (std::isalnum(x)) {
                return true;
            }
            const char acceptable_non_alphanumeric_characters[] = {'_'};
            for (char y : acceptable_non_alphanumeric_characters) {
                if (x == y) {
                    return true;
                }
            }
            return false;
        };
        return std::all_of(m_buffer.begin(), m_buffer.end(), is_acceptable_character);
    }

    bool holds_valid_literal_string_token() const {
        if (m_buffer.size() < 2) {
            return false;
        }
        // cannot have " in the middle (escaping is not supported)
        if (std::any_of(m_buffer.begin() + 1, m_buffer.end() - 1,
                        [](char c) { return c == '"'; })) {
            return false;
        }
        return m_buffer.front() == '"' && m_buffer.back() == '"';
    }

public:
    lexer_state() { m_buffer.reserve(50); }

    annotated_token read_token(source_location new_loc) {
        auto guard = helpers::make_simple_scope_guard([this, &new_loc]() {
            m_buffer.clear();
            m_loc = new_loc;
        });
        const auto& registry = const_token_registry();
        auto it = registry.find(std::string_view(m_buffer));
        if (it == registry.end()) {
            // special case: string literal
            if (holds_valid_literal_string_token()) {
                return {TOKEN_LITERAL_STRING, m_loc};
            }
            if (!holds_valid_identifier_token()) {
                throw std::runtime_error("Unknown token at " + std::string(m_loc));
            }
            // special case: consider this an identifier
            return {TOKEN_IDENTIFIER, m_loc};
        }
        return {it->second, m_loc};
    }

    void add(char c) { m_buffer.push_back(c); }
    void update(source_location loc) { m_loc = loc; }
    bool empty() const { return m_buffer.empty(); }

    void trim() { clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, m_buffer); }

    const std::string& buffer() const { return m_buffer; }
    source_location loc() const { return m_loc; }

    // flushes the buffer, asserting if buffer size != ExpectedSize
    template<std::size_t ExpectedSize> void flush() {
        assert((m_buffer.empty() || m_buffer.size() == ExpectedSize) &&
               "must not have called flush()");
        m_buffer.clear();
    }
};

namespace {
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

void read_token(std::iostream& out, lexer_state& state, source_location new_loc) {
    state.trim();
    if (!state.empty()) {
        out << std::string(state.read_token(new_loc));
        out << '\n';
    }
}

void try_read_token(std::iostream& out, char lookahead, lexer_state& state,
                    source_location new_loc) {
    static const auto delimiters = []() {
        const auto delim = [](token t) {
            const auto& registry = const_token_registry();
            auto it = std::find_if(registry.begin(), registry.end(),
                                   [&](const auto& p) { return p.second == t; });
            assert(registry.end() != it);
            return it->first.front();
        };

        return std::array{
#define TO_DELIMITER(NAME) delim(TOKEN_##NAME),
            CLSC_BES_FOR_EACH_DELIMITER_TOKEN(TO_DELIMITER)
#undef TO_DELIMITER

            // special characters
            '\0',
            '\n',
            '\r',
            '\t',
            ' ',
            '"',
            '\'',
        };
    }();

    const auto is_const_token = [](const std::string& pattern) {
        const auto& registry = const_token_registry();
        return registry.end() != registry.find(pattern);
    };

    const bool next_is_delimiter = std::any_of(std::begin(delimiters), std::end(delimiters),
                                               [&](char x) { return lookahead == x; });
    // collect as many characters as possible until a delimiter is coming next
    if (!next_is_delimiter) {
        // special case: even if there is no delimiter in the lookup, state
        // might immediately contain a valid constant token
        if (is_const_token(state.buffer())) {
            read_token(out, state, new_loc);
        }
        return;
    }

    // special case: if buffer + lookahead give valid constant token then wait
    // (e.g. resolves ASSIGN(=) vs EQ(==), OR(||), AND(&&), etc.)
    if (is_const_token(state.buffer() + lookahead)) {
        return;
    }

    read_token(out, state, new_loc);
}

}  // namespace

void lexer::tokenize() {
    lexer_state state{};

    int line = 0;
    int column = 0;

    for (char current = '\0'; m_in.get(current).good();) {
        state.add(current);
        ++column;

        switch (current) {
        case '\0': {
            assert(false && "should be rejected by the loop");
            break;
        }
        case '\n': {
            ++line;
            [[fallthrough]];
        }
        case '\r': {
            column = 0;
            state.flush<1>();  // try_read_token() should've emptied it
            state.update({line, column});
            continue;
        }
        case '(':
        case ')':
        case ';': {
            assert(state.buffer().size() <= 1);  // try_read_token() should've emptied it
            read_token(m_out, state, {line, column});
            continue;
        }
        case '\t':
        case ' ': {
            state.flush<1>();                       // try_read_token() should've emptied it
            column += spaces_for_tab(current) - 1;  // subtract 1 due to +1 in the beginning
            state.update({line, column});
            continue;
        }
        case '"': {
            process_literal_string(current, state, line, column);
            continue;
        }
        default: {
            if (char lookahead = m_in.peek(); lookahead != std::char_traits<char>::eof()) {
                try_read_token(m_out, lookahead, state, {line, column});
            }
            break;
        }
        }
    }

    // in case there's anything left in the buffer
    read_token(m_out, state, {line, column});
}

void lexer::process_literal_string(char start, lexer_state& state, int& line, int& column) {
    for (char current = '\0'; m_in.get(current).good();) {
        if (current == start) {
            state.add(current);
            ++column;
            read_token(m_out, state, {line, column});
            return;
        }

        // TODO: \\n and friends are fine
        switch (current) {
        case '\0':
        case '\n':
        case '\r':
        case '\t': {
            throw std::runtime_error("Invalid string literal at " + std::string(state.loc()));
            break;
        }
        default: {
            state.add(current);
            break;
        }
        }
        ++column;
    }
    // for-loop must find a symbol that terminates the string literal
    throw std::runtime_error("Invalid string literal at " + std::string(state.loc()));
}

}  // namespace bes
}  // namespace clsc
