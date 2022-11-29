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
#include "helpers.hpp"
#include "source_location.hpp"
#include "token_stream.hpp"
#include "tokens.hpp"

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <iostream>

/*!
%% tokens
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

%% grammar
program : statement_list

statement_list
 : statement
 | TOKEN_SEMICOLON statement_list

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

% TODO: requires lookahead(2) due to expression / assign_statement ambiguity
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

    static std::string stringify(label l) {
#define CASE(x)                                                                                    \
    case label::x:                                                                                 \
        return "<" #x ">"

        switch (l) {
            CASE(program);
            CASE(statement_list);
            CASE(statement);
            CASE(substatement);
            CASE(eval_statement);
            CASE(var_statement);
            CASE(assign_statement);
            CASE(alias_statement);
            CASE(substatement_expression);
            CASE(expression);
            CASE(single_token_expression);
            CASE(parenthesized_expression);
            CASE(not_statement);
        }
#undef CASE

        return "<unknown>";
    }

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

    operator std::string() const {
        if (std::holds_alternative<std::monostate>(m_content)) {
            return "<empty>";
        } else if (std::holds_alternative<terminal>(m_content)) {
            return std::string(std::get<terminal>(m_content));
        }
        return nonterminal::stringify(std::get<nonterminal_>(m_content)->get_label());
    }

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
    void visit(statement*) override { std::cout << "\tstatement\n"; }
    void visit(substatement*) override { std::cout << "\tsubstatement\n"; }
    void visit(eval_statement*) override { std::cout << "\teval_statement\n"; }
    void visit(var_statement*) override { std::cout << "\tvar_substatement\n"; }
    void visit(assign_statement*) override { std::cout << "\tassign_statement\n"; }
    void visit(alias_statement*) override { std::cout << "\talias_statement\n"; }
    void visit(substatement_expression*) override { std::cout << "\tsubstatement_expression\n"; }
    void visit(expression*) override { std::cout << "\texpression\n"; }
    void visit(single_token_expression*) override { std::cout << "\tsingle_token_expression\n"; }
    void visit(parenthesized_expression*) override { std::cout << "\tparenthesized_expression\n"; }
    void visit(not_statement*) override { std::cout << "\tnot_statement\n"; }
    ~printer_visitor() { std::cout << "... End of program\n"; }
};

template<typename T, typename U> T fast_cast(U* x) {
    assert(dynamic_cast<T>(x) != nullptr);
    return static_cast<T>(x);
}

[[noreturn]] inline void throw_parsing_error(std::string what) {
    throw std::runtime_error("Parsing error: " + what);
}

inline std::string expected_token_message(clsc::bes::token t) {
    return "Expected token <" + std::string(t) + ">";
}

std::string unexpected_token_error(const clsc::bes::source_location& loc,
                                   const clsc::bes::token& expected) {
    return "Unexpected token in BES expression at " + std::string(loc) + ". " +
           expected_token_message(expected);
}

// TODO: once ready, add [[nodiscard]]
template<size_t N>
std::array<clsc::bes::annotated_token, N> read_sequence(clsc::bes::token_stream in,
                                                        const clsc::bes::token (&sequence)[N]) {
    std::array<clsc::bes::annotated_token, N> tokens{};
    for (std::size_t i = 0; i < N; ++i) {
        if (!in.good()) {
            throw_parsing_error("input program ended unexpectedly. " +
                                expected_token_message(sequence[i]));
        }
        in.get(tokens[i]);
        if (tokens[i].tok != sequence[i]) {
            throw_parsing_error(unexpected_token_error(tokens[i].loc, sequence[i]));
        }
    }
    return tokens;
}

// returns whether token stream is empty or could be considered as such
bool consider_empty(clsc::bes::token_stream& in) {
    if (!in.good()) {
        return true;
    }
    return in.peek() == clsc::bes::TOKEN_SEMICOLON;
}

template<typename Stack>
using nonterminal_handler = void (*)(Stack&, nonterminal*, clsc::bes::token_stream&);
template<typename Stack>
nonterminal_handler<Stack> find_nonterminal_handler(parse_tree_nonterminal::label l,
                                                    clsc::bes::token_stream& in) {
#define NONTERMINAL_CASE(NAME)                                                                     \
    case parse_tree_nonterminal::label::NAME:                                                      \
        return [](Stack & stack [[maybe_unused]], nonterminal * This [[maybe_unused]],             \
                  clsc::bes::token_stream & in [[maybe_unused]])

    switch (l) {
        NONTERMINAL_CASE(program) { stack.emplace_back(std::make_unique<statement_list>()); };
        NONTERMINAL_CASE(statement_list) {
            const auto next_token = in.peek();
            if (next_token == clsc::bes::TOKEN_SEMICOLON) {
                clsc::bes::annotated_token dummy;
                in.get(dummy);
                stack.emplace_back(std::make_unique<statement_list>());
            } else {
                stack.emplace_back(std::make_unique<statement>());
            }
        };
        NONTERMINAL_CASE(single_token_expression) {  // TODO: delete this!
            auto next_token = in.peek();
            const auto suitable = {clsc::bes::TOKEN_IDENTIFIER, clsc::bes::TOKEN_LITERAL_TRUE,
                                   clsc::bes::TOKEN_LITERAL_FALSE};
            if (std::end(suitable) !=
                std::find(std::begin(suitable), std::end(suitable), next_token)) {
                in.get(fast_cast<single_token_expression*>(This)->token);
                // nothing to add to the stack here
                return;
            }
            auto sequence =
                clsc::helpers::join<char>(std::begin(suitable), std::end(suitable), ' ');
            throw_parsing_error("unexpected token. Expected one of {" + sequence + "}");
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
        };
        NONTERMINAL_CASE(assign_statement) {
            read_sequence(in, {clsc::bes::TOKEN_IDENTIFIER, clsc::bes::TOKEN_ASSIGN});
            stack.emplace_back(std::make_unique<substatement>());
        };
        NONTERMINAL_CASE(alias_statement) {
            read_sequence(in, {clsc::bes::TOKEN_ALIAS, clsc::bes::TOKEN_IDENTIFIER,
                               clsc::bes::TOKEN_ASSIGN, clsc::bes::TOKEN_LITERAL_STRING});
        };
        NONTERMINAL_CASE(var_statement) {
            read_sequence(in, {clsc::bes::TOKEN_VAR, clsc::bes::TOKEN_IDENTIFIER});
        };
        NONTERMINAL_CASE(eval_statement) {
            read_sequence(in, {clsc::bes::TOKEN_EVAL});
            stack.emplace_back(std::make_unique<substatement>());
        };
        NONTERMINAL_CASE(substatement) {
            if (consider_empty(in)) {
                return;
            }

            if (in.peek() == clsc::bes::TOKEN_NOT) {
                stack.emplace_back(std::make_unique<not_statement>());
            } else {
                stack.emplace_back(std::make_unique<expression>());
                stack.emplace_back(std::make_unique<substatement_expression>());
            }
        };
        NONTERMINAL_CASE(expression) {
            // expression
            //  : parenthesized_expression  - fallback
            //  | TOKEN_LITERAL_FALSE
            //  | TOKEN_LITERAL_TRUE
            //  | TOKEN_IDENTIFIER
            const auto next_token = in.peek();
            const auto suitable = {clsc::bes::TOKEN_IDENTIFIER, clsc::bes::TOKEN_LITERAL_TRUE,
                                   clsc::bes::TOKEN_LITERAL_FALSE};
            if (std::end(suitable) !=
                std::find(std::begin(suitable), std::end(suitable), next_token)) {
                // TODO: do something with this token?
                clsc::bes::annotated_token dummy;
                in.get(dummy);
                // nothing to add to the stack here
            } else {
                // fallback
                stack.emplace_back(std::make_unique<parenthesized_expression>());
            }
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
            if (consider_empty(in)) {
                return;
            }

            const auto next_token = in.peek();
            const auto parse_candidates = {
                clsc::bes::TOKEN_NEQ,         clsc::bes::TOKEN_EQ,  clsc::bes::TOKEN_ARROW_LEFT,
                clsc::bes::TOKEN_ARROW_RIGHT, clsc::bes::TOKEN_XOR, clsc::bes::TOKEN_AND,
                clsc::bes::TOKEN_OR,
            };
            const bool valid_parse =
                std::any_of(std::begin(parse_candidates), std::end(parse_candidates),
                            [&](auto x) { return next_token == x; });
            if (!valid_parse) {
                const auto sequence = clsc::helpers::join<char>(std::begin(parse_candidates),
                                                                std::end(parse_candidates), ' ');
                throw_parsing_error("unexpected token " + std::string(next_token) +
                                    ". Expected one of { " + sequence + " }");
            }
            // TODO: do something with this token
            clsc::bes::annotated_token dummy;
            in.get(dummy);

            stack.emplace_back(std::make_unique<expression>());
        };
        NONTERMINAL_CASE(parenthesized_expression) {
            stack.emplace_back(clsc::bes::TOKEN_PAREN_LEFT);
            stack.emplace_back(std::make_unique<substatement>());
            stack.emplace_back(clsc::bes::TOKEN_PAREN_RIGHT);
        };
        NONTERMINAL_CASE(not_statement) {
            read_sequence(in, {clsc::bes::TOKEN_NOT});
            stack.emplace_back(std::make_unique<expression>());
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
        std::reverse(std::begin(m_local), std::begin(m_local) + m_count);
        for (size_t i = 0; i < m_count; ++i) {
            auto& entry = m_local[i];
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
    unwrap(adaptor, current, in);
}

void create_ast(clsc::bes::ast::program&, clsc::bes::token_stream& in) {
    std::vector<parse_tree_element> stack;
    stack.emplace_back(std::make_unique<program>());

    printer_visitor printer;

    while (!stack.empty()) {
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
                    throw_parsing_error("unexpected token " + std::string(t) + ", expected " +
                                        std::string(x));
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

ast::program parser::parse() {
    ast::program p{};

    if (m_in.good()) {
        create_ast(p, m_in);
    }

    return p;
}

}  // namespace bes
}  // namespace clsc
