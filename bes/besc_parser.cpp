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
#include "token_stream.hpp"
#include "tokens.hpp"

#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <iostream>

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

% TODO: statement list with TOKEN_SEMICOLON in the middle is wrong?!
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
 : parenthesized_expression
 | TOKEN_LITERAL_FALSE
 | TOKEN_LITERAL_TRUE
 | TOKEN_IDENTIFIER

parenthesized_expression
 : TOKEN_PAREN_LEFT substatement TOKEN_PAREN_RIGHT

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

namespace {
struct parse_tree_visitor;

struct parse_tree_nonterminal {
    enum label {
        program,
        statement_list,
        statement,
        substatement,
        eval_statement,
        var_statement,
        assign_statement,
        alias_statement,
        substatement_expression,
        expression,
        single_token_expression,
        parenthesized_expression,
        not_statement,
    };

    virtual ~parse_tree_nonterminal() = default;
    virtual void apply(parse_tree_visitor* visitor) = 0;
    virtual label get_label() = 0;
};

using terminal = clsc::bes::token;
using nonterminal = parse_tree_nonterminal;

struct parse_tree_element {
    using nonterminal_ = std::unique_ptr<nonterminal>;

    parse_tree_element() = default;
    parse_tree_element(terminal x) : m_content(std::move(x)) {}
    parse_tree_element(nonterminal_ x) : m_content(std::move(x)) {}
    parse_tree_element(const parse_tree_element& x) = delete;
    parse_tree_element& operator=(const parse_tree_element& x) = delete;
    parse_tree_element(parse_tree_element&& x) = default;
    parse_tree_element& operator=(parse_tree_element&& x) = default;
    ~parse_tree_element() = default;

    template<typename... Fs> struct overloaded : Fs... { using Fs::operator()...; };
    // to be able to call overloaded( [] (...) {;}, ... )
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    operator bool() const { return std::holds_alternative<std::monostate>(m_content); }

    template<typename... Callables> void accept(Callables... calls) {
        std::visit(overloaded{calls...}, m_content);
    }

private:
    std::variant<std::monostate, terminal, nonterminal_> m_content;
};

struct program : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::program;
    }
};

struct statement_list : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::statement_list;
    }
};

struct statement : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::statement;
    }
};

struct substatement : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::substatement;
    }
};

struct eval_statement : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::eval_statement;
    }
};

struct var_statement : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::var_statement;
    }
};

struct assign_statement : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::assign_statement;
    }
};

struct alias_statement : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::alias_statement;
    }
};

struct substatement_expression : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::substatement_expression;
    }
};

struct expression : nonterminal {
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::expression;
    }
};

struct single_token_expression : nonterminal {  // true | false | identifier
    clsc::bes::annotated_token token;

    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::single_token_expression;
    }
};

struct parenthesized_expression : nonterminal {  // ( substatement )
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::parenthesized_expression;
    }
};

struct not_statement : nonterminal {  // ~ expression
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::not_statement;
    }
};

struct parse_tree_visitor {
    virtual ~parse_tree_visitor() = default;

    virtual void visit(program* This) {}
    virtual void visit(statement_list* This) {}
    virtual void visit(statement* This) {}
    virtual void visit(substatement* This) {}
    virtual void visit(eval_statement* This) {}
    virtual void visit(var_statement* This) {}
    virtual void visit(assign_statement* This) {}
    virtual void visit(alias_statement* This) {}
    virtual void visit(substatement_expression* This) {}
    virtual void visit(expression* This) {}
    virtual void visit(single_token_expression* This) {}
    virtual void visit(parenthesized_expression* This) {}
    virtual void visit(not_statement* This) {}
};

void program::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void statement_list::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void substatement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void eval_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void var_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void assign_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void alias_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void substatement_expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void single_token_expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void parenthesized_expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void not_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }

struct printer_visitor : parse_tree_visitor {
    printer_visitor() { std::cout << "Printing program ...\n"; }
    void visit(program*) override { std::cout << "\tprogram\n"; }
    void visit(statement_list*) override { std::cout << "\tstatement_list\n"; }
    void visit(single_token_expression*) override { std::cout << "\tsingle_token_expression\n"; }
    void visit(parenthesized_expression*) override { std::cout << "\tparenthesized_expression\n"; }
    void visit(not_statement*) override { std::cout << "\tnot_statement\n"; }
    ~printer_visitor() { std::cout << "... End of program\n"; }
};

template<typename T, typename U> T fast_cast(U* x) {
    assert(dynamic_cast<T>(x) != nullptr);
    return static_cast<T>(x);
}

template<typename Stack>
using nonterminal_handler = bool (*)(Stack&, nonterminal*, clsc::bes::token_stream&);
template<typename Stack>
nonterminal_handler<Stack> find_nonterminal_handler(parse_tree_nonterminal::label l,
                                                    clsc::bes::token_stream& in) {
#define NONTERMINAL_CASE(NAME)                                                                     \
    case parse_tree_nonterminal::label::NAME:                                                      \
        return [](Stack & stack [[maybe_unused]], nonterminal * This [[maybe_unused]],             \
                  clsc::bes::token_stream & in [[maybe_unused]]) -> bool

    switch (l) {
        NONTERMINAL_CASE(program) {
            stack.emplace_back(std::make_unique<statement_list>());
            return true;
        };
        NONTERMINAL_CASE(statement_list) {
            if (!in.good()) {  // <empty>
                return true;
            }
            stack.emplace_back(std::make_unique<statement>());
            stack.emplace_back(clsc::bes::TOKEN_SEMICOLON);
            stack.emplace_back(std::make_unique<statement_list>());
            return true;
        };
        NONTERMINAL_CASE(single_token_expression) {
            auto next_token = in.peek();
            const auto suitable = {clsc::bes::TOKEN_IDENTIFIER, clsc::bes::TOKEN_LITERAL_TRUE,
                                   clsc::bes::TOKEN_LITERAL_FALSE};
            if (std::end(suitable) !=
                std::find(std::begin(suitable), std::end(suitable), next_token)) {
                in.get(fast_cast<single_token_expression*>(This)->token);
                // nothing to add to the stack here
                return true;
            }
            return false;
        };
        NONTERMINAL_CASE(statement) {
            auto next_token = in.peek();
            // statement
            //  : substatement      - fallback
            //  | eval_statement    - TOKEN_EVAL ...
            //  | var_statement     - TOKEN_VAR ...
            //  | alias_statement   - TOKEN_ALIAS ...
            //  | assign_statement  - TOKEN_IDENTIFIER TOKEN_ASSIGN ...
            if (next_token == clsc::bes::TOKEN_IDENTIFIER &&
                in.peek2() == clsc::bes::TOKEN_ASSIGN) {
                stack.emplace_back(std::make_unique<assign_statement>());
            } else if (next_token == clsc::bes::TOKEN_ALIAS) {
                stack.emplace_back(std::make_unique<alias_statement>());
            } else if (next_token == clsc::bes::TOKEN_VAR) {
                stack.emplace_back(std::make_unique<var_statement>());
            } else if (next_token == clsc::bes::TOKEN_EVAL) {
                stack.emplace_back(std::make_unique<eval_statement>());
            } else {  // fallback
                stack.emplace_back(std::make_unique<substatement>());
            }
            return true;
        };
        NONTERMINAL_CASE(assign_statement) {
            stack.emplace_back(clsc::bes::TOKEN_IDENTIFIER);
            stack.emplace_back(clsc::bes::TOKEN_ASSIGN);
            stack.emplace_back(std::make_unique<substatement>());
            return true;
        };
        NONTERMINAL_CASE(alias_statement) {
            stack.emplace_back(clsc::bes::TOKEN_ALIAS);
            stack.emplace_back(clsc::bes::TOKEN_IDENTIFIER);
            stack.emplace_back(clsc::bes::TOKEN_ASSIGN);
            stack.emplace_back(clsc::bes::TOKEN_LITERAL_STRING);
            return true;
        };
        NONTERMINAL_CASE(var_statement) {
            stack.emplace_back(clsc::bes::TOKEN_VAR);
            stack.emplace_back(clsc::bes::TOKEN_IDENTIFIER);
            return true;
        };
        NONTERMINAL_CASE(eval_statement) {
            stack.emplace_back(clsc::bes::TOKEN_EVAL);
            stack.emplace_back(std::make_unique<substatement>());
            return true;
        };
        NONTERMINAL_CASE(substatement) {
            if (!in.good()) {  // <empty>
                return true;
            }

            if (in.peek() == clsc::bes::TOKEN_NOT) {
                stack.emplace_back(std::make_unique<not_statement>());
            } else {
                stack.emplace_back(std::make_unique<expression>());
                stack.emplace_back(std::make_unique<substatement_expression>());
            }

            return true;
        };
        NONTERMINAL_CASE(expression) {
            // expression
            //  : parenthesized_expression  - fallback
            //  | TOKEN_LITERAL_FALSE
            //  | TOKEN_LITERAL_TRUE
            //  | TOKEN_IDENTIFIER
            auto next_token = in.peek();
            if (next_token == clsc::bes::TOKEN_IDENTIFIER) {
                stack.emplace_back(clsc::bes::TOKEN_IDENTIFIER);
            } else if (next_token == clsc::bes::TOKEN_LITERAL_TRUE) {
                stack.emplace_back(clsc::bes::TOKEN_LITERAL_TRUE);
            } else if (next_token == clsc::bes::TOKEN_LITERAL_FALSE) {
                stack.emplace_back(clsc::bes::TOKEN_LITERAL_FALSE);
            } else {  // fallback
                stack.emplace_back(std::make_unique<parenthesized_expression>());
            }
            return true;
        };
        NONTERMINAL_CASE(substatement_expression) {
            // substatement_expression
            //  : TOKEN_OR expression
            //  | TOKEN_AND expression
            //  | TOKEN_XOR expression
            //  | TOKEN_ARROW_RIGHT expression
            //  | TOKEN_ARROW_LEFT expression
            //  | TOKEN_EQ expression
            //  | TOKEN_NEQ expression
            //  | <empty>
            if (!in.good()) {  // <empty>
                return true;
            }
            auto next_token = in.peek();
            const auto parse_candidates = {
                clsc::bes::TOKEN_NEQ,         clsc::bes::TOKEN_EQ,  clsc::bes::TOKEN_ARROW_LEFT,
                clsc::bes::TOKEN_ARROW_RIGHT, clsc::bes::TOKEN_XOR, clsc::bes::TOKEN_AND,
                clsc::bes::TOKEN_OR,
            };
            const bool valid_parse =
                std::any_of(std::begin(parse_candidates), std::end(parse_candidates),
                            [&](auto x) { return next_token == x; });
            if (!valid_parse) {
                // TODO: improve error to "expected one of: [ ... ]"
                throw std::runtime_error("Parsing error: unexpected token " +
                                         std::string(next_token));
            }

            stack.emplace_back(next_token.tok);
            stack.emplace_back(std::make_unique<expression>());
            return true;
        };
        NONTERMINAL_CASE(parenthesized_expression) {
            stack.emplace_back(clsc::bes::TOKEN_PAREN_LEFT);
            stack.emplace_back(std::make_unique<substatement>());
            stack.emplace_back(clsc::bes::TOKEN_PAREN_RIGHT);
            return true;
        };
        NONTERMINAL_CASE(not_statement) {
            stack.emplace_back(clsc::bes::TOKEN_NOT);
            stack.emplace_back(std::make_unique<expression>());
            return true;
        };
    }
#undef NONTERMINAL_CASE

    assert(false && "unreachable");
    return {};
}

template<typename Stack>
class ReverseOrderPush {  // Stack adaptor that pushes elements in reverse order
    std::array<typename Stack::value_type, 8> m_local;  // 8 is enough for local stack!
    size_t m_count{0};

    Stack& m_remote;

public:
    ReverseOrderPush(Stack& s) : m_remote(s) {}
    ~ReverseOrderPush() {
        // reverse received content before storing it into the (real) stack
        std::reverse(std::begin(m_local), std::end(m_local));
        for (auto& entry : m_local) {
            m_remote.emplace_back(std::move(entry));
        }
    }

    template<typename... Args> void emplace_back(Args&&... args) {
        assert(m_count < m_local.size());
        m_local[m_count] = typename Stack::value_type(std::forward<Args>(args)...);
        ++m_count;
    }
};

template<typename Stack>
void unwrap_nonterminal(Stack& stack, nonterminal* current, clsc::bes::token_stream& in) {
    // ensure that the stack is correctly pushed
    ReverseOrderPush<Stack> adaptor(stack);
    auto unwrap = find_nonterminal_handler<ReverseOrderPush<Stack>>(current->get_label(), in);
    auto succeeded = unwrap(adaptor, current, in);
    if (!succeeded) {
        throw std::runtime_error("Parsing error");
    }
}

void process_parse_tree(clsc::bes::ast::program&, clsc::bes::token_stream& in) {
    std::vector<parse_tree_element> stack;
    stack.emplace_back(std::make_unique<program>());

    printer_visitor printer;

    while (!stack.empty()) {
        if (!in.good()) {
            throw std::runtime_error("Parsing error: input program ended unexpectedly");
        }

        parse_tree_element current;
        using std::swap;
        swap(current, stack.back());
        stack.pop_back();

        // if:
        // * a terminal - verify the token stream
        // * a non-terminal - apply visitor and unwrap
        current.accept(
            [](std::monostate) {
                // do nothing
            },
            [&](const terminal& x) {
                clsc::bes::annotated_token t;
                in.get(t);
                if (t != x) {
                    throw std::runtime_error("Parsing error: unexpected token " + std::string(t) +
                                             ", expected " + std::string(x));
                }
            },
            [&](const std::unique_ptr<nonterminal>& x) {
                x->apply(&printer);
                unwrap_nonterminal(stack, x.get(), in);
            });
    }
}

}  // namespace

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

    if (m_in.good()) {
        process_parse_tree(p, m_in);
    }

    // while (m_in.good()) {
    //     auto expr = parse_expression();
    //     if (expr) {
    //         p.add(std::move(expr));
    //     }
    // }

    return p;
}

}  // namespace bes
}  // namespace clsc
