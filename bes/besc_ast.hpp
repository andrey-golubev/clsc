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

#pragma once

#include "source_location.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace clsc {
namespace bes {

class program_parser;

namespace ast {

struct base_visitor;

struct expression {
    expression(source_location loc) : m_loc(loc) {}
    virtual ~expression() = default;
    virtual void apply(base_visitor* visitor) = 0;

    source_location loc() const { return m_loc; }

protected:
    source_location m_loc;

private:
    virtual void add(std::unique_ptr<expression> e) = 0;
    // TODO: replace is *only* needed to relink binary expr's left child to its
    // binary expr's parent
    virtual void replace(const expression* const x, std::unique_ptr<expression> y) = 0;
    friend class ::clsc::bes::program_parser;
};

struct expression_list : expression {
    expression_list(source_location loc) : expression(loc) {}
    inline void apply(base_visitor* visitor) override;

protected:
    std::vector<std::unique_ptr<expression>> m_subexprs;

private:
    void add(std::unique_ptr<expression> e) override { m_subexprs.push_back(std::move(e)); }

    void replace(const expression* const x, std::unique_ptr<expression> y) override {
        auto old_it = std::find_if(
            m_subexprs.begin(),
            m_subexprs.end(),
            [&](const std::unique_ptr<expression>& e) { return e.get() == x; }
        );
        assert(old_it != m_subexprs.end());
        using std::swap;
        swap(*old_it, y);
    }
};

struct program {
    inline void apply(base_visitor* visitor);
    program(source_location loc) : m_all(std::make_unique<expression_list>(loc)) {}

private:
    std::unique_ptr<expression_list> m_all;

    friend class ::clsc::bes::program_parser;
};

struct identifier_expression : expression {
    identifier_expression(source_location loc, std::string_view name)
        : expression(loc), m_name(name) {}
    std::string_view name() const { return m_name; }
    inline void apply(base_visitor*) override;

private:
    std::string_view m_name;

    void add(std::unique_ptr<expression>) override { assert(false && "unexpected call to add()"); }

    void replace(const expression* const, std::unique_ptr<expression>) override {
        assert(false && "unexpected call to replace()");
    }
};

struct logical_binary_expression : expression {
    logical_binary_expression(source_location loc) : expression(loc) {}
    inline void apply(base_visitor* visitor) override;

    enum expr_kind {
        none,
        or_expr,
        and_expr,
        xor_expr,
        arrow_right_expr,
        arrow_left_expr,
        eq_expr,
        neq_expr,
    };

    const expression* left() const { return m_left.get(); }
    const expression* right() const { return m_right.get(); }
    expr_kind kind() const { return m_kind; }
    void set_kind(expr_kind kind) {
        assert(m_kind == none);
        assert(kind != none);
        m_kind = kind;
    }

private:
    std::unique_ptr<expression> m_left;
    std::unique_ptr<expression> m_right;
    expr_kind m_kind{none};

    void add(std::unique_ptr<expression> e) override {
        assert(!m_left || !m_right);
        if (!m_left) {
            m_left = std::move(e);
            return;
        }
        m_right = std::move(e);
    }

    void replace(const expression* const x, std::unique_ptr<expression> y) override {
        using std::swap;
        assert(m_left.get() == x || m_right.get() == x);
        if (m_left.get() == x) {
            swap(m_left, y);
            return;
        }
        swap(m_right, y);
    }

    friend class ::clsc::bes::program_parser;
};

struct not_expression : expression {
    not_expression(source_location loc) : expression(loc) {}
    inline void apply(base_visitor* visitor) override;

    const expression* expr() const { return m_expr.get(); }

private:
    std::unique_ptr<expression> m_expr;

    void add(std::unique_ptr<expression> e) override {
        assert(!m_expr);
        m_expr = std::move(e);
    }

    void replace(const expression* const x, std::unique_ptr<expression> y) override {
        assert(m_expr.get() == x);
        using std::swap;
        swap(m_expr, y);
    }
};

struct assign_expression : expression {
    assign_expression(source_location loc, std::unique_ptr<identifier_expression> i)
        : expression(loc), m_identifier(std::move(i)) {}
    inline void apply(base_visitor* visitor) override;

    const identifier_expression* identifier() const {
        return static_cast<identifier_expression*>(m_identifier.get());
    }
    const expression* value() const { return m_value.get(); }

private:
    std::unique_ptr<identifier_expression> m_identifier;
    std::unique_ptr<expression> m_value;

    void add(std::unique_ptr<expression> e) override {
        assert(!m_value);
        m_value = std::move(e);
    }

    void replace(const expression* const x, std::unique_ptr<expression> y) override {
        assert(m_value.get() == x);
        using std::swap;
        swap(m_value, y);
    }
};

struct alias_expression : expression {
    alias_expression(
        source_location loc, std::unique_ptr<identifier_expression> i, std::string_view lit
    )
        : expression(loc), m_identifier(std::move(i)), m_lit(lit) {}
    inline void apply(base_visitor* visitor) override;

    const identifier_expression* identifier() const {
        return static_cast<identifier_expression*>(m_identifier.get());
    }
    std::string_view literal() const { return m_lit; }

private:
    std::unique_ptr<identifier_expression> m_identifier;
    std::string_view m_lit{};

    void add(std::unique_ptr<expression>) override { assert(false && "unexpected call to add()"); }

    void replace(const expression* const, std::unique_ptr<expression>) override {
        assert(false && "unexpected call to replace()");
    }
};

struct var_expression : expression {
    var_expression(source_location loc, std::unique_ptr<identifier_expression> i)
        : expression(loc), m_identifier(std::move(i)) {}
    inline void apply(base_visitor* visitor) override;

    const identifier_expression* identifier() const {
        return static_cast<identifier_expression*>(m_identifier.get());
    }

private:
    std::unique_ptr<identifier_expression> m_identifier;

    void add(std::unique_ptr<expression>) override { assert(false && "unexpected call to add()"); }

    void replace(const expression* const, std::unique_ptr<expression>) override {
        assert(false && "unexpected call to replace()");
    }
};

struct eval_expression : expression {
    eval_expression(source_location loc) : expression(loc) {}
    inline void apply(base_visitor* visitor) override;

    const expression* expr() const { return m_expr.get(); }

private:
    std::unique_ptr<expression> m_expr;

    void add(std::unique_ptr<expression> e) override {
        assert(!m_expr);
        m_expr = std::move(e);
    }

    void replace(const expression* const x, std::unique_ptr<expression> y) override {
        assert(m_expr.get() == x);
        using std::swap;
        swap(m_expr, y);
    }
};

struct parenthesized_expression : expression {
    parenthesized_expression(source_location loc) : expression(loc) {}
    inline void apply(base_visitor* visitor) override;

    const expression* expr() const { return m_expr.get(); }

private:
    std::unique_ptr<expression> m_expr;

    void add(std::unique_ptr<expression> e) override {
        assert(!m_expr);
        m_expr = std::move(e);
    }

    void replace(const expression* const x, std::unique_ptr<expression> y) override {
        assert(m_expr.get() == x);
        using std::swap;
        swap(m_expr, y);
    }
};

struct bool_literal_expression : expression {
    bool_literal_expression(source_location loc, bool v) : expression(loc), m_value(v) {}
    inline void apply(base_visitor* visitor) override;
    bool value() const { return m_value; }

private:
    bool m_value = false;

    void add(std::unique_ptr<expression> e) override {
        assert(false && "unexpected call to add()");
    }

    void replace(const expression* const, std::unique_ptr<expression>) override {
        assert(false && "unexpected call to replace()");
    }
};

struct base_visitor {
    virtual ~base_visitor() = default;

    [[nodiscard]] virtual bool visit(program*) { return true; }
    virtual void post_visit(program*) {}
    [[nodiscard]] virtual bool visit(expression_list*) { return true; }
    virtual void post_visit(expression_list*) {}
    [[nodiscard]] virtual bool visit(identifier_expression*) { return true; }
    virtual void post_visit(identifier_expression*) {}
    [[nodiscard]] virtual bool visit(logical_binary_expression*) { return true; }
    virtual void post_visit(logical_binary_expression*) {}
    [[nodiscard]] virtual bool visit(not_expression*) { return true; }
    virtual void post_visit(not_expression*) {}
    [[nodiscard]] virtual bool visit(assign_expression*) { return true; }
    virtual void post_visit(assign_expression*) {}
    [[nodiscard]] virtual bool visit(alias_expression*) { return true; }
    virtual void post_visit(alias_expression*) {}
    [[nodiscard]] virtual bool visit(var_expression*) { return true; }
    virtual void post_visit(var_expression*) {}
    [[nodiscard]] virtual bool visit(eval_expression*) { return true; }
    virtual void post_visit(eval_expression*) {}
    [[nodiscard]] virtual bool visit(parenthesized_expression*) { return true; }
    virtual void post_visit(parenthesized_expression*) {}
    [[nodiscard]] virtual bool visit(bool_literal_expression*) { return true; }
    virtual void post_visit(bool_literal_expression*) {}
};

inline void program::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_all);
        m_all->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void expression_list::apply(base_visitor* visitor) {
    if (!visitor->visit(this))
        return;
    for (const auto& expr : m_subexprs) {
        assert(expr);
        expr->apply(visitor);
    }
    visitor->post_visit(this);
}

inline void identifier_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this))
        visitor->post_visit(this);
}

inline void logical_binary_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_left);
        assert(m_right);
        m_left->apply(visitor);
        m_right->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void not_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_expr);
        m_expr->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void assign_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_identifier);
        assert(m_value);
        m_identifier->apply(visitor);
        m_value->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void alias_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_identifier);
        m_identifier->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void var_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_identifier);
        m_identifier->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void eval_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_expr);
        m_expr->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void parenthesized_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this)) {
        assert(m_expr);
        m_expr->apply(visitor);
        visitor->post_visit(this);
    }
}

inline void bool_literal_expression::apply(base_visitor* visitor) {
    if (visitor->visit(this))
        visitor->post_visit(this);
}

}  // namespace ast
}  // namespace bes
}  // namespace clsc
