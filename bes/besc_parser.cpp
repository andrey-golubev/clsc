// Copyright 2023 Andrey Golubev
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

#include <iostream>  // TODO: remove this once debugging is over

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
program : statement_list % this is a purely conceptual thing

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
 : expression substatement_expression % possible binary expression
 | <empty>

eval_statement : TOKEN_EVAL substatement

var_statement : TOKEN_VAR TOKEN_IDENTIFIER

% TODO: requires lookahead(2) due to expression / assign_statement ambiguity
assign_statement : TOKEN_IDENTIFIER TOKEN_ASSIGN substatement

alias_statement : TOKEN_ALIAS TOKEN_IDENTIFIER TOKEN_ASSIGN TOKEN_LITERAL_STRING

expression
 : parenthesized_expression
 | not_expression
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

not_expression : TOKEN_NOT expression

 */

namespace {

struct parse_tree_visitor;

struct parse_tree_nonterminal {
    enum label {
        statement_list,
        statement,
        substatement,
        eval_statement,
        var_statement,
        assign_statement,
        alias_statement,
        substatement_expression,
        expression,
        parenthesized_expression,
        not_expression,
    };

    static std::string stringify(label l) {
#define CASE(x)                                                                                    \
    case label::x:                                                                                 \
        return "<" #x ">"

        switch (l) {
            CASE(statement_list);
            CASE(statement);
            CASE(substatement);
            CASE(eval_statement);
            CASE(var_statement);
            CASE(assign_statement);
            CASE(alias_statement);
            CASE(substatement_expression);
            CASE(expression);
            CASE(parenthesized_expression);
            CASE(not_expression);
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
    parse_tree_element(std::monostate) : parse_tree_element() {}
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

    template<typename T> bool holds_alternative() const noexcept {
        return std::holds_alternative<T>(m_content);
    }

private:
    std::variant<std::monostate, terminal, nonterminal_> m_content;
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

struct parenthesized_expression : nonterminal {  // ( substatement )
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::parenthesized_expression;
    }
};

struct not_expression : nonterminal {  // ~ expression
    void apply(parse_tree_visitor* visitor) override;
    parse_tree_nonterminal::label get_label() override {
        return parse_tree_nonterminal::label::not_expression;
    }
};

struct parse_tree_visitor {
    virtual ~parse_tree_visitor() = default;

    virtual void visit(statement_list* This) {}
    virtual void visit(statement* This) {}
    virtual void visit(substatement* This) {}
    virtual void visit(eval_statement* This) {}
    virtual void visit(var_statement* This) {}
    virtual void visit(assign_statement* This) {}
    virtual void visit(alias_statement* This) {}
    virtual void visit(substatement_expression* This) {}
    virtual void visit(expression* This) {}
    virtual void visit(parenthesized_expression* This) {}
    virtual void visit(not_expression* This) {}
};

void statement_list::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void substatement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void eval_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void var_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void assign_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void alias_statement::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void substatement_expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void parenthesized_expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }
void not_expression::apply(parse_tree_visitor* visitor) { visitor->visit(this); }

struct printer_visitor : parse_tree_visitor {
    printer_visitor() { std::cout << "Start of program ...\n"; }
    void visit(statement_list*) override { std::cout << "\tstatement_list\n"; }
    void visit(statement*) override { std::cout << "\tstatement\n"; }
    void visit(substatement*) override { std::cout << "\tsubstatement\n"; }
    void visit(eval_statement*) override { std::cout << "\teval_statement\n"; }
    void visit(var_statement*) override { std::cout << "\tvar_substatement\n"; }
    void visit(assign_statement*) override { std::cout << "\tassign_statement\n"; }
    void visit(alias_statement*) override { std::cout << "\talias_statement\n"; }
    void visit(substatement_expression*) override { std::cout << "\tsubstatement_expression\n"; }
    void visit(expression*) override { std::cout << "\texpression\n"; }
    void visit(parenthesized_expression*) override { std::cout << "\tparenthesized_expression\n"; }
    void visit(not_expression*) override { std::cout << "\not_expression\n"; }
    ~printer_visitor() { std::cout << "... End of program\n"; }
};

template<typename T, typename U> T fast_cast(U* x) {
    assert(dynamic_cast<T>(x) != nullptr);
    return static_cast<T>(x);
}

[[noreturn]] inline void throw_parsing_error(std::string what) {
    throw std::runtime_error("Parsing error: " + what);
}

[[noreturn]] inline void throw_internal_parsing_error(std::string what) {
    throw std::runtime_error("Internal parsing error: " + what);
}

template<typename T> T expression_cast(clsc::bes::ast::expression* x) {
    auto y = dynamic_cast<T>(x);
    if (!y) {
        throw_internal_parsing_error("unexpected expression type");
    }
    return y;
}

inline std::string expected_token_message(clsc::bes::token t) {
    return "Expected token <" + std::string(t) + ">";
}

std::string
unexpected_token_error(const clsc::bes::source_location& loc, const clsc::bes::token& expected) {
    return "Unexpected token in BES expression at " + std::string(loc) + ". " +
           expected_token_message(expected);
}

template<size_t N>
[[nodiscard]] std::array<clsc::bes::annotated_token, N>
read_sequence(clsc::bes::token_stream& in, const clsc::bes::token (&sequence)[N]) {
    std::array<clsc::bes::annotated_token, N> tokens{};
    for (std::size_t i = 0; i < N; ++i) {
        if (!in.good()) {
            throw_parsing_error(
                "input program ended unexpectedly. " + expected_token_message(sequence[i])
            );
        }
        in.get(tokens[i]);
        if (tokens[i].tok != sequence[i]) {
            throw_parsing_error(unexpected_token_error(tokens[i].loc, sequence[i]));
        }
    }
    return tokens;
}

template<size_t N>
clsc::bes::source_location
location_from_sequence(const std::array<clsc::bes::annotated_token, N>& sequence) {
    static_assert(N > 0, "cannot create source location for an empty sequence");
    clsc::bes::source_location loc = sequence[0].loc;
    for (size_t i = 1; i < N; ++i) {
        loc.length += sequence[i].loc.length;
    }
    return loc;
}

// returns whether token stream is empty or could be considered as such
bool consider_empty(
    clsc::bes::token_stream& in, std::initializer_list<clsc::bes::token> extra_markers = {}
) {
    if (!in.good()) {
        return true;
    }
    const auto this_token = in.peek();
    // Note: TOKEN_SEMICOLON is obviously special
    return this_token == clsc::bes::TOKEN_SEMICOLON ||
           (std::end(extra_markers) !=
            std::find(std::begin(extra_markers), std::end(extra_markers), this_token));
}

template<typename Stack>
class reverse_order_push {  // Stack adaptor that pushes elements in reverse order
    std::array<typename Stack::value_type, 8> m_local;  // 8 is enough for local stack!
    size_t m_count{0};

    Stack& m_remote;

public:
    reverse_order_push(Stack& s) : m_remote(s) {}
    ~reverse_order_push() {
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
    const typename Stack::value_type& back() const {
        if (m_count == 0) {
            return m_remote.back();
        }
        return m_local[m_count - 1];
    }
    void pop_back() {
        if (m_count == 0) {
            m_remote.pop_back();
            return;
        }
        --m_count;
    }
    bool empty() const { return m_count == 0 && m_remote.empty(); }
};
template<typename Stack> reverse_order_push(Stack) -> reverse_order_push<Stack>;

template<typename Stack> typename Stack::value_type pop(Stack& stack) {
    assert(!stack.empty());
    typename Stack::value_type last{};
    using std::swap;
    swap(last, stack.back());
    stack.pop_back();
    return last;
}

}  // namespace

namespace clsc {
namespace bes {

class program_parser {
    token_stream& m_in;
    std::string_view m_program;
    ast::program m_ast;

    using amend_callback = void (*)(ast::expression*, std::unique_ptr<ast::expression>);
    struct expression_stack_element {
        ast::expression* content{nullptr};
        void add(std::unique_ptr<ast::expression> e) {
            assert(content);
            assert(e);
            content->add(std::move(e));
        }
    };
    std::vector<expression_stack_element> m_expression_stack;

    // TODO: remove this in favor of a general solution
    static void relink_children(ast::logical_binary_expression* src, ast::expression* dst) {
        assert(!src->m_right);  // otherwise it is indeed a binary expression!
        if (!src->m_left) {
            return;
        }
        dst->replace(src, std::move(src->m_left));
    }

    // TODO: the below types should really be hidden inside the ast_creator
    using shrink_sentinel = std::monostate;
    enum amend_policy {  // TODO: remove - this is only useful for debugging right now
        amend_arbitrarily = 0,
        amend_top_level = 1,
    };
    enum shrink_policy {
        just_shrink = 0,
        drop_sentinel = 1,
    };
    template<typename Stack> class ast_creator {
        Stack& m_parse_tree_stack;
        program_parser& m_parser;

    public:
        ast_creator(Stack& s, program_parser* p) : m_parse_tree_stack(s), m_parser(*p) {}

        /*! simultaneously adds \a e as a child to the current ast stack head, and
            puts \a e on top of the stack

           \sa amend_ast_head(), shrink_ast()
         */
        void grow_ast(std::unique_ptr<ast::expression> e) {
            assert(e);
            auto object = e.get();
            amend_ast_head(std::move(e));
            m_parser.m_expression_stack.push_back(expression_stack_element{object});

            m_parse_tree_stack.emplace_back(shrink_sentinel{});
        }

        /*! adds \a child as a child to to the current ast stack head

            \sa grow_ast(), shrink_ast()
         */
        void amend_ast_head(
            std::unique_ptr<ast::expression> child, amend_policy policy = amend_arbitrarily
        ) {
            assert(!m_parser.m_expression_stack.empty());
            assert(m_parser.m_expression_stack.back().content);
            assert(child);
            if (policy == amend_top_level)
                assert(m_parser.m_expression_stack.size() == 1);

            m_parser.m_expression_stack.back().add(std::move(child));
        }

        /*! removes the node that is currently at the top of the ast stack,
            processing its children beforehand in the order they were added

            \sa grow_ast(), amend_ast_head()
         */
        expression_stack_element shrink_ast(shrink_policy policy = just_shrink) {
            assert(!m_parser.m_expression_stack.empty());
            auto element = pop(m_parser.m_expression_stack);
            if (policy == just_shrink) {
                return element;
            }

            assert(policy == drop_sentinel);  // special
            assert(!m_parse_tree_stack.empty());
            assert(m_parse_tree_stack.back().template holds_alternative<shrink_sentinel>());
            m_parse_tree_stack.pop_back();

            // relink popped element's children to the new stack head
            auto binary_expr = fast_cast<ast::logical_binary_expression*>(element.content);
            assert(!m_parser.m_expression_stack.empty());
            program_parser::relink_children(
                binary_expr, m_parser.m_expression_stack.back().content
            );
            // note: due to the nature of this whole operation, we could no
            // longer use the popped element
            return {nullptr};
        }
    };
    template<typename Stack> ast_creator(Stack) -> ast_creator<Stack>;

    std::string_view find_substring(source_location loc) const {
        return m_program.substr(loc.offset, loc.length);
    }

    template<typename Stack>
    using nonterminal_handler = void (*)(program_parser*, Stack&, clsc::bes::token_stream&);
    template<typename Stack>
    nonterminal_handler<Stack>
    find_nonterminal_handler(parse_tree_nonterminal::label l, clsc::bes::token_stream& in) {
#define NONTERMINAL_CASE(NAME)                                                                     \
    case parse_tree_nonterminal::label::NAME:                                                      \
        return                                                                                     \
            [](program_parser * parser [[maybe_unused]],                                           \
               Stack & stack [[maybe_unused]],                                                     \
               clsc::bes::token_stream & in [[maybe_unused]])

        switch (l) {
            NONTERMINAL_CASE(statement_list) {
                const auto this_token = in.peek();
                if (this_token == clsc::bes::TOKEN_SEMICOLON) {
                    // Note: TOKEN_SEMICOLON is ignored here
                    clsc::bes::annotated_token dummy;
                    in.get(dummy);
                    stack.emplace_back(std::make_unique<statement_list>());
                } else {
                    stack.emplace_back(std::make_unique<statement>());
                }
            };
            NONTERMINAL_CASE(statement) {
                const auto this_token = in.peek();
                // statement
                //  : substatement      - fallback
                //  | eval_statement    - TOKEN_EVAL ...
                //  | var_statement     - TOKEN_VAR ...
                //  | alias_statement   - TOKEN_ALIAS ...
                //  | assign_statement  - TOKEN_IDENTIFIER TOKEN_ASSIGN ...
                if (this_token == clsc::bes::TOKEN_IDENTIFIER &&
                    in.peek2() == clsc::bes::TOKEN_ASSIGN) {
                    stack.emplace_back(std::make_unique<assign_statement>());
                } else if (this_token == clsc::bes::TOKEN_ALIAS) {
                    stack.emplace_back(std::make_unique<alias_statement>());
                } else if (this_token == clsc::bes::TOKEN_VAR) {
                    stack.emplace_back(std::make_unique<var_statement>());
                } else if (this_token == clsc::bes::TOKEN_EVAL) {
                    stack.emplace_back(std::make_unique<eval_statement>());
                } else {  // fallback
                    stack.emplace_back(std::make_unique<substatement>());
                }
            };
            NONTERMINAL_CASE(assign_statement) {
                const auto valid =
                    read_sequence(in, {clsc::bes::TOKEN_IDENTIFIER, clsc::bes::TOKEN_ASSIGN});
                stack.emplace_back(std::make_unique<substatement>());

                // ast:
                const auto& id_token = valid[0];
                auto identifier_expr = std::make_unique<ast::identifier_expression>(
                    id_token.loc, parser->find_substring(id_token.loc)
                );

                auto assign_expr = std::make_unique<ast::assign_expression>(
                    location_from_sequence(valid), std::move(identifier_expr)
                );

                ast_creator(stack, parser).grow_ast(std::move(assign_expr));
            };
            NONTERMINAL_CASE(alias_statement) {
                const auto valid = read_sequence(
                    in,
                    {clsc::bes::TOKEN_ALIAS,
                     clsc::bes::TOKEN_IDENTIFIER,
                     clsc::bes::TOKEN_ASSIGN,
                     clsc::bes::TOKEN_LITERAL_STRING}
                );

                // ast:
                const auto& id_token = valid[1];
                auto identifier_expr = std::make_unique<ast::identifier_expression>(
                    id_token.loc, parser->find_substring(id_token.loc)
                );

                const auto& literal_token = valid[3];
                auto alias_expr = std::make_unique<ast::alias_expression>(
                    location_from_sequence(valid),
                    std::move(identifier_expr),
                    parser->find_substring(literal_token.loc)
                );

                ast_creator(stack, parser).amend_ast_head(std::move(alias_expr), amend_top_level);
            };
            NONTERMINAL_CASE(var_statement) {
                const auto valid =
                    read_sequence(in, {clsc::bes::TOKEN_VAR, clsc::bes::TOKEN_IDENTIFIER});

                // ast:
                const auto& id_token = valid[1];
                auto identifier_expr = std::make_unique<ast::identifier_expression>(
                    id_token.loc, parser->find_substring(id_token.loc)
                );

                auto var_expr = std::make_unique<ast::var_expression>(
                    location_from_sequence(valid), std::move(identifier_expr)
                );

                ast_creator(stack, parser).amend_ast_head(std::move(var_expr), amend_top_level);
            };
            NONTERMINAL_CASE(eval_statement) {
                const auto valid = read_sequence(in, {clsc::bes::TOKEN_EVAL});
                stack.emplace_back(std::make_unique<substatement>());

                // ast:
                auto eval_expr =
                    std::make_unique<ast::eval_expression>(location_from_sequence(valid));
                ast_creator(stack, parser).grow_ast(std::move(eval_expr));
            };
            NONTERMINAL_CASE(substatement) {
                if (consider_empty(in)) {
                    return;
                }

                stack.emplace_back(std::make_unique<expression>());
                stack.emplace_back(std::make_unique<substatement_expression>());

                // ast:
                auto loc = in.peek().loc;
                auto maybe_binary_expr = std::make_unique<ast::logical_binary_expression>(loc);
                ast_creator(stack, parser).grow_ast(std::move(maybe_binary_expr));
            };
            NONTERMINAL_CASE(expression) {
                // expression
                //  : parenthesized_expression  - fallback
                //  | not_expression
                //  | TOKEN_LITERAL_FALSE
                //  | TOKEN_LITERAL_TRUE
                //  | TOKEN_IDENTIFIER
                const auto this_token = in.peek();
                const auto suitable = {
                    clsc::bes::TOKEN_IDENTIFIER,
                    clsc::bes::TOKEN_LITERAL_TRUE,
                    clsc::bes::TOKEN_LITERAL_FALSE};
                if (std::end(suitable) !=
                    std::find(std::begin(suitable), std::end(suitable), this_token)) {
                    clsc::bes::annotated_token single_token;
                    in.get(single_token);

                    // ast:
                    if (single_token == clsc::bes::TOKEN_IDENTIFIER) {
                        auto identifier_expr = std::make_unique<ast::identifier_expression>(
                            single_token.loc, parser->find_substring(single_token.loc)
                        );
                        ast_creator(stack, parser).amend_ast_head(std::move(identifier_expr));
                        return;
                    }
                    auto bool_expr = std::make_unique<ast::bool_literal_expression>(
                        single_token.loc, single_token == clsc::bes::TOKEN_LITERAL_TRUE
                    );
                    ast_creator(stack, parser).amend_ast_head(std::move(bool_expr));
                } else if (this_token == clsc::bes::TOKEN_NOT) {
                    stack.emplace_back(std::make_unique<not_expression>());
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
                if (consider_empty(in, {clsc::bes::TOKEN_PAREN_RIGHT})) {
                    // previously (in substatement parsing) we pushed a binary
                    // expression onto the ast stack. that expression is not in
                    // fact valid in this particular case, so we must drop it
                    auto element = ast_creator(stack, parser).shrink_ast(drop_sentinel);
                    assert(element.content == nullptr);
                    return;
                }

                const auto this_token = in.peek();
                const auto parse_candidates = {
                    clsc::bes::TOKEN_NEQ,
                    clsc::bes::TOKEN_EQ,
                    clsc::bes::TOKEN_ARROW_LEFT,
                    clsc::bes::TOKEN_ARROW_RIGHT,
                    clsc::bes::TOKEN_XOR,
                    clsc::bes::TOKEN_AND,
                    clsc::bes::TOKEN_OR,
                };
                const bool valid_parse = std::any_of(
                    std::begin(parse_candidates),
                    std::end(parse_candidates),
                    [&](auto x) { return this_token == x; }
                );
                if (!valid_parse) {
                    const auto sequence = clsc::helpers::join<char>(
                        std::begin(parse_candidates), std::end(parse_candidates), ' '
                    );
                    throw_parsing_error(
                        "unexpected token " + std::string(this_token) + ". Expected one of { " +
                        sequence + " }"
                    );
                }

                clsc::bes::annotated_token expr_token;
                in.get(expr_token);

                stack.emplace_back(std::make_unique<expression>());

                // ast:
                auto synthetic_expr = [&]() -> std::unique_ptr<ast::expression> {
                    if (expr_token == clsc::bes::TOKEN_NEQ)
                        return std::make_unique<ast::neq_synthetic>();
                    if (expr_token == clsc::bes::TOKEN_EQ)
                        return std::make_unique<ast::eq_synthetic>();
                    if (expr_token == clsc::bes::TOKEN_ARROW_LEFT)
                        return std::make_unique<ast::arrow_left_synthetic>();
                    if (expr_token == clsc::bes::TOKEN_ARROW_RIGHT)
                        return std::make_unique<ast::arrow_right_synthetic>();
                    if (expr_token == clsc::bes::TOKEN_XOR)
                        return std::make_unique<ast::xor_synthetic>();
                    if (expr_token == clsc::bes::TOKEN_AND)
                        return std::make_unique<ast::and_synthetic>();
                    if (expr_token == clsc::bes::TOKEN_OR)
                        return std::make_unique<ast::or_synthetic>();

                    assert(false && "unreachable due to verification step above");
                    return nullptr;
                }();
                ast_creator(stack, parser).amend_ast_head(std::move(synthetic_expr));
            };
            NONTERMINAL_CASE(parenthesized_expression) {
                stack.emplace_back(clsc::bes::TOKEN_PAREN_LEFT);
                stack.emplace_back(std::make_unique<substatement>());
                stack.emplace_back(clsc::bes::TOKEN_PAREN_RIGHT);

                // ast:
                const auto left_paren_token = in.peek();
                auto paren_expr =
                    std::make_unique<ast::parenthesized_expression>(left_paren_token.loc);
                ast_creator(stack, parser).grow_ast(std::move(paren_expr));
            };
            NONTERMINAL_CASE(not_expression) {
                const auto valid = read_sequence(in, {clsc::bes::TOKEN_NOT});
                stack.emplace_back(std::make_unique<expression>());

                // ast:
                const auto not_token = valid[0];
                auto not_expr = std::make_unique<ast::not_expression>(not_token.loc);
                ast_creator(stack, parser).grow_ast(std::move(not_expr));
            };
        }
#undef NONTERMINAL_CASE

        assert(false && "unreachable");
        return {};
    }

    template<typename Stack>
    void unwrap_nonterminal(Stack& stack, nonterminal* current, clsc::bes::token_stream& in) {
        // ensure that the stack is correctly pushed
        reverse_order_push adaptor(stack);
        auto unwrap = find_nonterminal_handler<reverse_order_push<Stack>>(current->get_label(), in);
        unwrap(this, adaptor, m_in);
    }

public:
    program_parser(token_stream& in, std::string_view p)
        : m_in(in), m_program(p), m_ast({0, 0, 0, m_program.size()}) {
        m_expression_stack.push_back(expression_stack_element{m_ast.m_all.get()});
    }

    ast::program extract_ast() {
        assert(!m_in.good());

        assert(m_expression_stack.size() == 1);
        m_expression_stack.pop_back();

        // restore construction invariants
        ast::program ast = std::exchange(m_ast, ast::program({0, 0, 0, m_program.size()}));
        m_expression_stack.push_back(expression_stack_element{m_ast.m_all.get()});

        return ast;
    }

    void parse() {
        std::vector<parse_tree_element> stack;
        stack.emplace_back(std::make_unique<statement_list>());

        printer_visitor printer;

        while (!stack.empty()) {
            parse_tree_element current = pop(stack);

            // if:
            // * a sentinel (std::monostate) - do special operation
            // * a terminal - verify the token stream
            // * a non-terminal - apply visitor and unwrap
            current.accept(
                [&stack, this](shrink_sentinel) {
                    // shrink the ast stack as we hit the sentinel
                    ast_creator(stack, this).shrink_ast();
                },
                [&](const terminal& x) {
                    if (!m_in.good()) {
                        throw_parsing_error("no token in the stream, expected " + std::string(x));
                    }

                    clsc::bes::annotated_token t;
                    m_in.get(t);
                    if (t != x) {
                        throw_parsing_error(
                            "unexpected token " + std::string(t) + ", expected " + std::string(x)
                        );
                    }
                },
                [&](const std::unique_ptr<nonterminal>& x) {
                    x->apply(&printer);
                    unwrap_nonterminal(stack, x.get(), m_in);
                }
            );

            // special case: the stack is empty but the token stream is not. it
            // means we have parsed a single entity of a statement list (a
            // statement), so we need to carry on
            if (stack.empty() && m_in.good()) {
                stack.emplace_back(std::make_unique<statement_list>());
            }
        }

        assert(!m_in.good() && "the token stream must have been exhausted by the parsing");
    }
};

ast::program parser::parse() {
    program_parser parsing_driver{m_in, m_raw_program};
    parsing_driver.parse();
    return parsing_driver.extract_ast();
}

}  // namespace bes
}  // namespace clsc
