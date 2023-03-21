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

#include "besc_ast.hpp"
#include "source_location.hpp"

#include <sstream>

namespace clsc {
namespace bes {

struct scope;
struct semant {
    semant(ast::program& program);
    ~semant();

    struct semantic_error {
        source_location loc;
        std::string description;

        friend bool operator==(const semantic_error& x, const semantic_error& y) noexcept {
            return x.loc == y.loc && x.description == y.description;
        }
        friend bool operator!=(const semantic_error& x, const semantic_error& y) noexcept {
            return !(x == y);
        }
    };

    template<typename... DescriptionBits>
    static semantic_error make_error(source_location loc, DescriptionBits&&... bits) {
        std::stringstream ss;
        (ss << ... << bits) << "\n";
        return {loc, ss.str()};
    }

    [[nodiscard]] std::vector<semantic_error> analyze();

private:
    ast::program& m_program;
    std::unique_ptr<scope> m_global;
};

}  // namespace bes
}  // namespace clsc
