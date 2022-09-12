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

#include <cstddef>
#include <memory>
#include <vector>

namespace clsc {
namespace bes {
namespace ast {

struct base_visitor;

struct expression {
    expression(source_location loc) : m_loc(loc) {}
    virtual void apply(base_visitor* visitor);

    source_location loc() { return m_loc; }
    void add(std::unique_ptr<expression> e) { m_subexprs.push_back(std::move(e)); }

protected:
    source_location m_loc;
    std::vector<std::unique_ptr<expression>> m_subexprs;
};

struct program {
    void apply(base_visitor* visitor);
    void add(std::unique_ptr<expression> e) { m_exprs.push_back(std::move(e)); }

private:
    std::vector<std::unique_ptr<expression>> m_exprs;
};

struct semicolon_expression : expression {
    semicolon_expression(source_location loc) : expression(loc) {}
    void apply(base_visitor*) override {}  // do nothing
};

struct identifier_expression : expression {
    identifier_expression(source_location loc, std::string_view name)
        : expression(loc), m_name(name) {}
    std::string_view name() const { return m_name; }
    void apply(base_visitor*) override {}  // do nothing

private:
    std::string_view m_name;
};

struct logical_binary_expression : expression {
    logical_binary_expression(source_location loc, std::unique_ptr<expression> l,
                              std::unique_ptr<expression> r)
        : expression(loc) {
        add(std::move(l));
        add(std::move(r));
    }
    void apply(base_visitor* visitor) override;

    const expression* left() const { return m_subexprs[0].get(); }
    const expression* right() const { return m_subexprs[1].get(); }
};

struct or_expression : logical_binary_expression {};

struct and_expression : logical_binary_expression {};

struct not_expression : expression {
    not_expression(source_location loc, std::unique_ptr<expression> e) : expression(loc) {
        add(std::move(e));
    }
    void apply(base_visitor* visitor) override;
    const expression* expr() const { return m_subexprs[0].get(); }
};

struct xor_expression : logical_binary_expression {};

struct arrow_right_expression : logical_binary_expression {};

struct arrow_left_expression : logical_binary_expression {};

struct eq_expression : logical_binary_expression {};

struct neq_expression : logical_binary_expression {};

struct assign_expression : expression {
    assign_expression(source_location loc, std::unique_ptr<identifier_expression> l,
                      std::unique_ptr<expression> r)
        : expression(loc) {
        add(std::move(l));
        add(std::move(r));
    }
    void apply(base_visitor* visitor) override;

    const identifier_expression* identifier() const {
        return static_cast<identifier_expression*>(m_subexprs[0].get());
    }
    const expression* right() const { return m_subexprs[1].get(); }
};

struct alias_expression : expression {
    alias_expression(source_location loc, std::unique_ptr<identifier_expression> i,
                     std::string_view lit)
        : expression(loc), m_lit(lit) {
        add(std::move(i));
    }
    void apply(base_visitor* visitor) override;

    const identifier_expression* identifier() const {
        return static_cast<identifier_expression*>(m_subexprs[0].get());
    }
    std::string_view literal() const { return m_lit; }

private:
    std::string_view m_lit{};
};

struct var_expression : expression {
    var_expression(source_location loc, std::unique_ptr<identifier_expression> i)
        : expression(loc) {
        add(std::move(i));
    }
    void apply(base_visitor* visitor) override;

    const identifier_expression* identifier() const {
        return static_cast<identifier_expression*>(m_subexprs[0].get());
    }
};

struct eval_expression : expression {
    eval_expression(source_location loc, std::unique_ptr<expression> e) : expression(loc) {
        add(std::move(e));
    }
    void apply(base_visitor* visitor) override;
    const expression* expr() const { return m_subexprs[0].get(); }
};

struct bool_literal_expression : expression {
    bool_literal_expression(source_location loc, bool v) : expression(loc), m_value(v) {}
    void apply(base_visitor* visitor) override;
    bool value() const { return m_value; }

private:
    bool m_value = false;
};

struct base_visitor {
    virtual bool visit(program*) { return true; }
    virtual void postVisit(program*) {}
    virtual bool visit(or_expression*) { return true; }
    virtual void postVisit(or_expression*) {}
    virtual bool visit(and_expression*) { return true; }
    virtual void postVisit(and_expression*) {}
    virtual bool visit(not_expression*) { return true; }
    virtual void postVisit(not_expression*) {}
    virtual bool visit(xor_expression*) { return true; }
    virtual void postVisit(xor_expression*) {}
    virtual bool visit(arrow_right_expression*) { return true; }
    virtual void postVisit(arrow_right_expression*) {}
    virtual bool visit(arrow_left_expression*) { return true; }
    virtual void postVisit(arrow_left_expression*) {}
    virtual bool visit(eq_expression*) { return true; }
    virtual void postVisit(eq_expression*) {}
    virtual bool visit(neq_expression*) { return true; }
    virtual void postVisit(neq_expression*) {}
    virtual bool visit(assign_expression*) { return true; }
    virtual void postVisit(assign_expression*) {}
    virtual bool visit(alias_expression*) { return true; }
    virtual void postVisit(alias_expression*) {}
    virtual bool visit(var_expression*) { return true; }
    virtual void postVisit(var_expression*) {}
    virtual bool visit(eval_expression*) { return true; }
    virtual void postVisit(eval_expression*) {}
    virtual bool visit(bool_literal_expression*) { return true; }
    virtual void postVisit(bool_literal_expression*) {}
};

}  // namespace ast
}  // namespace bes
}  // namespace clsc
