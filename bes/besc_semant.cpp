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

#include "besc_semant.hpp"
#include "besc_ast.hpp"
#include "source_location.hpp"

#include <string>
#include <unordered_map>
#include <variant>

namespace clsc {
namespace bes {

// TODO: this likely has to be returned, so that IR generator could use this
// information.
struct scope {
    struct symbol {
        std::string name;
        source_location loc;
        std::variant<std::monostate, std::string, const ast::expression*> content;

        symbol(const ast::identifier_expression* id)
            : name(std::string(id->name())), loc(id->loc()) {}

        template<typename T>
        symbol(const ast::identifier_expression* id, T extra)
            : name(std::string(id->name())), loc(id->loc()), content(extra) {}
    };

private:
    // TODO: the key is essentially symbol's name
    std::unordered_map<std::string_view, symbol> m_symbols;

    [[nodiscard]] auto add(const ast::identifier_expression* id, symbol s) {
        assert(!id->name().empty());
        assert(!s.name.empty());
        return m_symbols.emplace(id->name(), s);
    }

public:
    // add variable
    [[nodiscard]] auto add(const ast::identifier_expression* id) {
        assert(id);
        return add(id, symbol{id});
    }
    // add alias
    [[nodiscard]] auto add(const ast::identifier_expression* id, std::string alias_string) {
        assert(id);
        return add(id, symbol{id, alias_string});
    }
    // add assignment
    [[nodiscard]] auto
    add(const ast::identifier_expression* id, const ast::expression* assign_expr) {
        assert(id);
        return add(id, symbol{id, assign_expr});
    }

    auto find(const ast::identifier_expression* id) const { return m_symbols.find(id->name()); }

    bool exists(const ast::identifier_expression* id) const { return find(id) != m_symbols.end(); }
};

namespace {
class semantic_checker : public ast::base_visitor {
    scope* global_scope;
    std::vector<semant::semantic_error> errors;

    void check_for_redeclaration(
        bool added, const ast::identifier_expression* id, source_location original
    ) {
        if (added)
            return;
        errors.push_back(semant::make_error(
            id->loc(), id->name(), " is redeclared. First declared at ", std::string(original)
        ));
    }

public:
    semantic_checker(scope* scope) : global_scope(scope) { assert(global_scope); }

    bool visit(ast::identifier_expression* id) override {
        assert(id);
        if (!global_scope->exists(id)) {
            errors.push_back(
                semant::make_error(id->loc(), id->name(), " is used before declaration")
            );
        }
        return true;
    }

    bool visit(ast::var_expression* var) override {
        auto [pos, result] = global_scope->add(var->identifier());
        check_for_redeclaration(result, var->identifier(), pos->second.loc);
        return true;
    }

    bool visit(ast::alias_expression* alias) override {
        assert(!alias->literal().empty());
        auto [pos, result] = global_scope->add(alias->identifier(), std::string(alias->literal()));
        check_for_redeclaration(result, alias->identifier(), pos->second.loc);
        return true;
    }

    bool visit(ast::assign_expression* assign) override {
        assert(assign->value());
        auto [pos, result] = global_scope->add(assign->identifier(), assign->value());
        check_for_redeclaration(result, assign->identifier(), pos->second.loc);
        return true;
    }

    std::vector<semant::semantic_error> take_errors() { return std::move(errors); }
};
}  // namespace

semant::semant(ast::program& program) : m_program(program), m_global(std::make_unique<scope>()) {}
semant::~semant() = default;
std::vector<semant::semantic_error> semant::analyze() {
    semantic_checker checker(m_global.get());
    m_program.apply(&checker);
    return checker.take_errors();
}

}  // namespace bes
}  // namespace clsc
