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
#include "tokens.hpp"

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

% grammar (note: character ';' denotes TOKEN_SEMICOLON)
expr ::=
| TOKEN_PAREN_LEFT expr TOKEN_PAREN_RIGHT ;
| expr TOKEN_OR expr ;
| expr TOKEN_AND expr ;
| TOKEN_NOT expr ;
| expr TOKEN_XOR expr ;
| expr TOKEN_ARROW_RIGHT expr ;
| expr TOKEN_ARROW_LEFT expr ;
| expr TOKEN_EQ expr ;
| expr TOKEN_NEQ expr ;
| TOKEN_IDENTIFIER TOKEN_ASSIGN expr ;  % TODO: do we need this?
| TOKEN_ALIAS TOKEN_IDENTIFIER TOKEN_ASSIGN TOKEN_LITERAL_STRING ;
| TOKEN_VAR TOKEN_IDENTIFIER ;
| TOKEN_EVAL expr ;
| TOKEN_IDENTIFIER
| TOKEN_LITERAL_TRUE
| TOKEN_LITERAL_FALSE
 */

namespace clsc {
namespace bes {

ast::program parser::parse() {
    ast::program p{};
    return p;
}

}  // namespace bes
}  // namespace clsc
