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

#include "besc_parser.hpp"
#include "besc_ast.hpp"
#include "source_location.hpp"
#include "tokens.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/*!
% tokens
TOKEN_UNKNOWN
TOKEN_OR                ||
TOKEN_AND               &&
TOKEN_NOT               ~
TOKEN_XOR               ^
TOKEN_ARROW_RIGHT       ->
TOKEN_ARROW_LEFT        <-
TOKEN_EQ                ==
TOKEN_NEQ               !=
TOKEN_ASSIGN            =
TOKEN_ALIAS             symbol
TOKEN_VAR               var
TOKEN_EVAL              eval
TOKEN_SEMICOLON         ;
TOKEN_IDENTIFIER
TOKEN_LITERAL_TRUE      true
TOKEN_LITERAL_FALSE     false
TOKEN_LITERAL_STRING
TOKEN_PAREN_LEFT        (
TOKEN_PAREN_RIGHT       )

% grammar
program : statement_list

statement_list
 : statement TOKEN_SEMICOLON statement_list
 | <empty>

statement
 : substatement
 | eval_statement
 | var_statement
 | assign_statement
 | alias_statement

substatement
 : expression substatement_expression
 | not_statement
 | <empty>

eval_statement : TOKEN_EVAL substatement

var_statement : TOKEN_VAR TOKEN_IDENTIFIER

% TODO: this might require lookahead(2) due to expression / assign_statement ambiguity
assign_statement : TOKEN_IDENTIFIER TOKEN_ASSIGN substatement

alias_statement : TOKEN_ALIAS TOKEN_IDENTIFIER TOKEN_ASSIGN TOKEN_LITERAL_STRING

expression
 : TOKEN_PAREN_LEFT substatement TOKEN_PAREN_RIGHT
 | TOKEN_LITERAL_FALSE
 | TOKEN_LITERAL_TRUE
 | TOKEN_IDENTIFIER

substatement_expression
 : TOKEN_OR expression
 | TOKEN_AND expression
 | TOKEN_XOR expression
 | TOKEN_ARROW_RIGHT expression
 | TOKEN_ARROW_LEFT expression
 | TOKEN_EQ expression
 | TOKEN_NEQ expression
 | <empty>

not_statement : TOKEN_NOT expression

 */

namespace clsc {
namespace bes {

namespace {
std::string unexpected_termination_error(const source_location& loc) {
    return "Unexpected termination when parsing BES expression at " + std::string(loc);
}

std::string unexpected_token_error(const source_location& loc, const token& expected) {
    return "Unexpected token in BES expression at " + std::string(loc) +
           " expected: " + std::string(expected);
}

template<size_t N>
std::array<annotated_token, N> read_sequence(token_stream in, source_location loc,
                                             const token (&sequence)[N]) {
    std::array<annotated_token, N> tokens{};
    for (std::size_t i = 0; i < N; ++i) {
        if (!in.good()) {
            throw std::runtime_error(unexpected_termination_error(loc));
        }
        in.get(tokens[i]);
        if (tokens[i].tok != sequence[i]) {
            throw std::runtime_error(unexpected_token_error(tokens[i].loc, sequence[i]));
        }
    }
    return tokens;
}
}  // namespace

std::unique_ptr<ast::expression> parser::parse_expression() {
    if (!m_in.good()) {
        return {};
    }

    annotated_token t;
    m_in.get(t);

    switch (t.tok.id) {
    // base cases
    case token::SEMICOLON: {
        return std::make_unique<ast::semicolon_expression>(t.loc);
    }
    case token::LITERAL_TRUE: {
        return std::make_unique<ast::bool_literal_expression>(t.loc, true);
    }
    case token::LITERAL_FALSE: {
        return std::make_unique<ast::bool_literal_expression>(t.loc, false);
    }
    case token::IDENTIFIER: {
        annotated_token lookahead = m_in.peek();
        if (lookahead.tok == TOKEN_ASSIGN) {
            // TOKEN_IDENTIFIER TOKEN_ASSIGN expr ;  // TODO: do we need this?

            // TODO: handle this branch correctly
            (void)read_sequence(m_in, t.loc, {TOKEN_ASSIGN});
            auto expr = parse_expression();
            (void)read_sequence(m_in, t.loc, {TOKEN_SEMICOLON});
            return {};
        }
        std::string_view name(m_raw_program.data() + t.loc.offset, t.loc.length);
        return std::make_unique<ast::identifier_expression>(t.loc, name);
    }

    case token::OR: {
        break;
    }
    case token::AND: {
        break;
    }
    case token::NOT: {
        // TOKEN_NOT expr ;
        auto expr = parse_expression();
        (void)read_sequence(m_in, t.loc, {TOKEN_SEMICOLON});
        return std::make_unique<ast::not_expression>(t.loc, std::move(expr));
    }
    case token::XOR: {
        break;
    }
    case token::ARROW_RIGHT: {
        break;
    }
    case token::ARROW_LEFT: {
        break;
    }
    case token::EQ: {
        break;
    }
    case token::NEQ: {
        break;
    }
    case token::ASSIGN: {
        break;
    }
    case token::ALIAS: {
        // TOKEN_ALIAS TOKEN_IDENTIFIER TOKEN_ASSIGN TOKEN_LITERAL_STRING ;
        auto tokens = read_sequence(
            m_in, t.loc, {TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_LITERAL_STRING, TOKEN_SEMICOLON});

        std::string_view name(m_raw_program.data() + tokens[0].loc.offset, tokens[0].loc.length);
        auto id_expr = std::make_unique<ast::identifier_expression>(tokens[0].loc, name);
        std::string_view string_literal(m_raw_program.data() + tokens[2].loc.offset,
                                        tokens[2].loc.length);
        return std::make_unique<ast::alias_expression>(t.loc, std::move(id_expr), string_literal);
    }
    case token::VAR: {
        // TOKEN_VAR TOKEN_IDENTIFIER ;
        auto tokens = read_sequence(m_in, t.loc, {TOKEN_IDENTIFIER, TOKEN_SEMICOLON});

        std::string_view name(m_raw_program.data() + tokens[0].loc.offset, tokens[0].loc.length);
        auto id_expr = std::make_unique<ast::identifier_expression>(tokens[0].loc, name);
        return std::make_unique<ast::var_expression>(t.loc, std::move(id_expr));
    }
    case token::EVAL: {
        // TOKEN_EVAL expr ;
        auto expr = parse_expression();
        (void)read_sequence(m_in, t.loc, {TOKEN_SEMICOLON});
        return std::make_unique<ast::eval_expression>(t.loc, std::move(expr));
    }
    case token::LITERAL_STRING: {
        break;
    }
    case token::PAREN_LEFT: {
        break;
    }
    case token::PAREN_RIGHT: {
        break;
    }
    default:
        throw std::runtime_error("Unexpected BES expression at " + std::string(t.loc));
    }

    return {};
}

ast::program parser::parse() {
    ast::program p{};

    while (m_in.good()) {
        auto expr = parse_expression();
        if (expr) {
            p.add(std::move(expr));
        }
    }

    return p;
}

}  // namespace bes
}  // namespace clsc
