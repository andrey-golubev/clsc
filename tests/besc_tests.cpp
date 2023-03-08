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

#include "besc_lexer.hpp"
#include "besc_parser.hpp"
#include "helpers.hpp"
#include "tokens.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <utility>

struct test_name_corrector {
    std::string operator()(std::string test_name) {
        // remove trailing '_'
        while (!test_name.empty() && test_name[test_name.size() - 1] == '_') {
            test_name.pop_back();
        }
        // ensure non-empty name
        if (test_name.empty()) {
            test_name = "EMPTY";
        }

        auto it = repeats.find(test_name);
        if (it == repeats.end()) {
            repeats.insert({test_name, 1});
            return test_name;
        }
        test_name += std::to_string(it->second++);
        return test_name;
    }

private:
    std::unordered_map<std::string, std::size_t> repeats;
};

TEST(besc_lexer_tests_basic, creatable) {
    clsc::bes::token_stream tokout;
    clsc::bes::lexer lexer{std::cin, tokout};
    (void)lexer;
}

struct besc_lexer_tests : testing::TestWithParam<std::pair<std::string, std::string>> {
    static auto test_name_printer() {
        static test_name_corrector correct{};
        using TestParamInfo = testing::TestParamInfo<std::pair<std::string, std::string>>;
        return [](const TestParamInfo& info) {
            const auto& output = info.param.second;
            std::string test_name;
            test_name.reserve(output.size());
            for (char c : output) {
                if (std::isalpha(c) || c == '_') {
                    test_name.push_back(c);
                }
                if (c == '\n') {
                    test_name.push_back('_');
                    test_name.push_back('_');
                }
            }

            // deduplicate and all that
            return correct(std::move(test_name));
        };
    }
};

TEST_P(besc_lexer_tests, all) {
    const std::pair<std::string, std::string>& in_out = GetParam();

    std::stringstream ssin;
    clsc::bes::token_stream tokout;
    ssin << in_out.first;

    auto input = ssin.str();
    clsc::bes::lexer lexer(ssin, tokout);
    try {
        lexer.tokenize();
    } catch (const std::exception& e) {
        FAIL() << "Tokenizing \"" << input << "\" failed with: " << e.what() << std::endl;
    }

    std::stringstream ssout;
    ssout << tokout;
    EXPECT_EQ(in_out.second, ssout.str());
}

INSTANTIATE_TEST_CASE_P(
    standalone_tokens,
    besc_lexer_tests,
    testing::Values(
        std::make_pair("||", "OR 0:0\n"),
        std::make_pair("&&", "AND 0:0\n"),
        std::make_pair("~", "NOT 0:0\n"),
        std::make_pair("^", "XOR 0:0\n"),
        std::make_pair("->", "ARROW_RIGHT 0:0\n"),
        std::make_pair("<-", "ARROW_LEFT 0:0\n"),
        std::make_pair("==", "EQ 0:0\n"),
        std::make_pair("!=", "NEQ 0:0\n"),
        std::make_pair("=", "ASSIGN 0:0\n"),
        std::make_pair("symbol", "ALIAS 0:0\n"),
        std::make_pair("var", "VAR 0:0\n"),
        std::make_pair("eval", "EVAL 0:0\n"),
        std::make_pair(";", "SEMICOLON 0:0\n"),
        std::make_pair("x", "IDENTIFIER 0:0\n"),
        std::make_pair("_90iyu", "IDENTIFIER 0:0\n"),
        std::make_pair("true", "LITERAL_TRUE 0:0\n"),
        std::make_pair("false", "LITERAL_FALSE 0:0\n"),
        std::make_pair("\"hello! world\"", "LITERAL_STRING 0:1\n"),
        std::make_pair("(", "PAREN_LEFT 0:0\n"),
        std::make_pair(")", "PAREN_RIGHT 0:0\n"),

        // special cases
        std::make_pair("", ""),
        std::make_pair(" ", ""),
        std::make_pair("\t", "")
    ),
    besc_lexer_tests::test_name_printer()
);

INSTANTIATE_TEST_CASE_P(
    many_tokens,
    besc_lexer_tests,
    testing::Values(
        std::make_pair("_x==_01y", "IDENTIFIER 0:0\nEQ 0:2\nIDENTIFIER 0:4\n"),
        std::make_pair("symbol x;", "ALIAS 0:0\nIDENTIFIER 0:7\nSEMICOLON 0:8\n"),
        std::make_pair("_x == _01y", "IDENTIFIER 0:0\nEQ 0:3\nIDENTIFIER 0:6\n"),
        std::make_pair(
            "symbol x = \"foo && bar\";",
            "ALIAS 0:0\nIDENTIFIER 0:7\nASSIGN 0:9\nLITERAL_STRING 0:12\nSEMICOLON 0:23\n"
        ),
        std::make_pair(
            "( x || y ) && z;",
            "PAREN_LEFT 0:0\nIDENTIFIER 0:2\nOR 0:4\nIDENTIFIER 0:7\nPAREN_RIGHT "
            "0:9\nAND 0:11\nIDENTIFIER 0:14\nSEMICOLON 0:15\n"
        ),

        // multi-line
        std::make_pair(
            "symbol x=\"A || B\";\neval x;",
            "ALIAS 0:0\nIDENTIFIER 0:7\nASSIGN 0:8\nLITERAL_STRING 0:10\nSEMICOLON 0:17\nEVAL "
            "1:0\nIDENTIFIER 1:5\nSEMICOLON 1:6\n"
        )
    ),
    besc_lexer_tests::test_name_printer()
);

clsc::bes::annotated_token annotated(clsc::bes::token t) {
    return clsc::bes::annotated_token{t, {}};
}

TEST(besc_parser_tests_basic, creatable) {
    clsc::bes::token_stream tokin;
    tokin << annotated(clsc::bes::TOKEN_LITERAL_TRUE);
    std::string raw_program = "true";
    clsc::bes::parser p{tokin, raw_program};
    (void)p;
}

struct besc_parser_tests : testing::TestWithParam<std::pair<clsc::bes::token_stream, std::string>> {
    static auto test_name_printer() {
        static test_name_corrector correct{};
        using TestParamInfo =
            testing::TestParamInfo<std::pair<clsc::bes::token_stream, std::string>>;
        return [](const TestParamInfo& info) {
            auto copy = info.param.first;
            std::string test_name;
            while (copy.good()) {
                clsc::bes::annotated_token t;
                copy.get(t);
                test_name += std::string(t.tok) + "__";
            }

            // deduplicate and all that
            return correct(std::move(test_name));
        };
    }
};

TEST_P(besc_parser_tests, all) {
    auto param = GetParam();
    auto& tokin = param.first;
    clsc::bes::parser parser{tokin, std::move(param.second)};
    (void)parser.parse();
}

INSTANTIATE_TEST_CASE_P(
    simple_tokens,
    besc_parser_tests,
    testing::Values(
        std::make_pair<clsc::bes::token_stream>({annotated(clsc::bes::TOKEN_SEMICOLON)}, ";"),
        std::make_pair<clsc::bes::token_stream>({annotated(clsc::bes::TOKEN_IDENTIFIER)}, "x"),
        std::make_pair<clsc::bes::token_stream>(
            {annotated(clsc::bes::TOKEN_IDENTIFIER), annotated(clsc::bes::TOKEN_SEMICOLON)},
            "M_231K0_sd;"
        ),
        std::make_pair<clsc::bes::token_stream>(
            {annotated(clsc::bes::TOKEN_LITERAL_TRUE), annotated(clsc::bes::TOKEN_SEMICOLON)},
            "true;"
        ),
        std::make_pair<clsc::bes::token_stream>(
            {annotated(clsc::bes::TOKEN_LITERAL_FALSE), annotated(clsc::bes::TOKEN_SEMICOLON)},
            "false;"
        ),
        std::make_pair<clsc::bes::token_stream>(
            {annotated(clsc::bes::TOKEN_PAREN_LEFT),
             annotated(clsc::bes::TOKEN_IDENTIFIER),
             annotated(clsc::bes::TOKEN_PAREN_RIGHT)},
            "(_1239jxq_1)"
        ),
        std::make_pair<clsc::bes::token_stream>(
            {annotated(clsc::bes::TOKEN_PAREN_LEFT),
             annotated(clsc::bes::TOKEN_PAREN_LEFT),
             annotated(clsc::bes::TOKEN_IDENTIFIER),
             annotated(clsc::bes::TOKEN_PAREN_RIGHT),
             annotated(clsc::bes::TOKEN_PAREN_RIGHT)},
            "((x))"
        ),
        std::make_pair<clsc::bes::token_stream>(
            {annotated(clsc::bes::TOKEN_NOT), annotated(clsc::bes::TOKEN_IDENTIFIER)}, "~x"
        )
    ),
    besc_parser_tests::test_name_printer()
);

struct besc_lexer_parser_tests : testing::TestWithParam<std::pair<std::string, std::string>> {

    class ast_dumper : public clsc::bes::ast::base_visitor {
    private:
        std::ostream& m_out;
        size_t m_indentation{0};

        template<typename T> std::ostream& operator<<(T&& expr) {
            m_out << std::string(m_indentation, ' ') << std::forward<T>(expr);
            // Note: returning ostream guarantees that we enter this function
            //       only once per '<<' chain
            return m_out;
        }

        void common_post_visit(std::string_view ending = "};\n") {
            m_indentation--;
            *this << ending;
        }

    public:
        ast_dumper(std::ostream& os) : m_out(os) {}

        bool visit(clsc::bes::ast::program* p) override {
            *this << "program {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::program*) override { common_post_visit("}"); }

        bool visit(clsc::bes::ast::expression_list* el) override {
            *this << "expression_list(" << el->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::expression_list*) override { common_post_visit("}\n"); }

        bool visit(clsc::bes::ast::identifier_expression* ie) override {
            *this << "id(" << ie->loc() << "): " << ie->name() << ";\n";
            return true;
        }

        bool visit(clsc::bes::ast::logical_binary_expression* lbe) override {
            const char* kind = [&]() {
                switch (lbe->kind()) {
                case clsc::bes::ast::logical_binary_expression::or_expr:
                    return "or";
                case clsc::bes::ast::logical_binary_expression::and_expr:
                    return "and";
                case clsc::bes::ast::logical_binary_expression::xor_expr:
                    return "xor";
                case clsc::bes::ast::logical_binary_expression::arrow_right_expr:
                    return "arrow_right";
                case clsc::bes::ast::logical_binary_expression::arrow_left_expr:
                    return "arrow_left";
                case clsc::bes::ast::logical_binary_expression::eq_expr:
                    return "equal";
                case clsc::bes::ast::logical_binary_expression::neq_expr:
                    return "not_equal";
                default:
                    assert(false && "invalid logical binary expression");
                }
                return "";
            }();
            *this << kind << "(" << lbe->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::logical_binary_expression*) override {
            common_post_visit();
        }

        bool visit(clsc::bes::ast::not_expression* ne) override {
            *this << "not(" << ne->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::not_expression*) override { common_post_visit(); }

        bool visit(clsc::bes::ast::assign_expression* ae) override {
            *this << "assign(" << ae->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::assign_expression*) override { common_post_visit(); }

        bool visit(clsc::bes::ast::alias_expression* ae) override {
            *this << "alias(" << ae->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::alias_expression* ae) override {
            *this << "'" << ae->literal() << "';\n";
            common_post_visit();
        }

        bool visit(clsc::bes::ast::var_expression* ve) override {
            *this << "var(" << ve->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::var_expression*) override { common_post_visit(); }

        bool visit(clsc::bes::ast::eval_expression* ee) override {
            *this << "eval(" << ee->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::eval_expression*) override { common_post_visit(); }

        bool visit(clsc::bes::ast::parenthesized_expression* pe) override {
            *this << "parenthesized(" << pe->loc() << ") {\n";
            m_indentation++;
            return true;
        }
        void post_visit(clsc::bes::ast::parenthesized_expression*) override { common_post_visit(); }

        bool visit(clsc::bes::ast::bool_literal_expression* ble) override {
            *this << "bool(" << ble->loc() << "): " << std::boolalpha << ble->value() << ";\n";
            return true;
        }
    };
};

TEST_P(besc_lexer_parser_tests, all) {
    const auto& param = GetParam();

    std::stringstream in_stream;
    in_stream << param.first;
    clsc::bes::token_stream token_stream;
    clsc::bes::lexer lexer{in_stream, token_stream};
    lexer.tokenize();
    ASSERT_FALSE(in_stream.good());

    auto program = param.first;
    clsc::bes::parser parser{token_stream, std::move(program)};
    auto ast = parser.parse();

    std::stringstream ss;
    ast_dumper dumper(ss);
    ast.apply(&dumper);

    auto expected = param.second;
    clsc::helpers::trim(clsc::helpers::trim_both_sides_tag{}, expected);
    ASSERT_EQ(ss.str(), expected);
}

INSTANTIATE_TEST_CASE_P(
    valid_programs,
    besc_lexer_parser_tests,
    testing::Values(
        std::make_pair(
            R"(
x || y;
y && x;
x ^ y;
x -> y;
x <- y;
x == y;
x != y;
~x;
)",
            R"(
program {
 expression_list(0:0) {
  or(1:0) {
   id(1:0): x;
   id(1:5): y;
  };
  and(2:0) {
   id(2:0): y;
   id(2:5): x;
  };
  xor(3:0) {
   id(3:0): x;
   id(3:4): y;
  };
  arrow_right(4:0) {
   id(4:0): x;
   id(4:5): y;
  };
  arrow_left(5:0) {
   id(5:0): x;
   id(5:5): y;
  };
  equal(6:0) {
   id(6:0): x;
   id(6:5): y;
  };
  not_equal(7:0) {
   id(7:0): x;
   id(7:5): y;
  };
  not(8:0) {
   id(8:1): x;
  };
 }
})"
        ),

        std::make_pair(
            "(x || y) && x;",
            R"(
program {
 expression_list(0:0) {
  and(0:0) {
   parenthesized(0:0) {
    or(0:1) {
     id(0:1): x;
     id(0:6): y;
    };
   };
   id(0:12): x;
  };
 }
})"
        ),
        std::make_pair("eval x && y;", R"(
program {
 expression_list(0:0) {
  eval(0:0) {
   and(0:5) {
    id(0:5): x;
    id(0:10): y;
   };
  };
 }
})"),
        std::make_pair("var My_veryBest_var;", R"(
program {
 expression_list(0:0) {
  var(0:0) {
   id(0:4): My_veryBest_var;
  };
 }
})"),
        std::make_pair("x = true", R"(
program {
 expression_list(0:0) {
  assign(0:0) {
   id(0:0): x;
   bool(0:4): true;
  };
 }
})"),
        std::make_pair("symbol long_expr = \"foo && (bar || baz)\";", R"(
program {
 expression_list(0:0) {
  alias(0:0) {
   id(0:7): long_expr;
   'foo && (bar || baz)';
  };
 }
})"),

        // multi-line and complicated
        std::make_pair(
            R"(
symbol long_expr = "foo && (bar || baz) ";
var x;
x = long_expr || y;
)",
            R"(
program {
 expression_list(0:0) {
  alias(1:0) {
   id(1:7): long_expr;
   'foo && (bar || baz) ';
  };
  var(2:0) {
   id(2:4): x;
  };
  assign(3:0) {
   id(3:0): x;
   or(3:4) {
    id(3:4): long_expr;
    id(3:17): y;
   };
  };
 }
})"
        ),

        // parentheses and negation
        std::make_pair("(~x)", R"(
program {
 expression_list(0:0) {
  parenthesized(0:0) {
   not(0:1) {
    id(0:2): x;
   };
  };
 }
})"),
        std::make_pair("~(x)", R"(
program {
 expression_list(0:0) {
  not(0:0) {
   parenthesized(0:1) {
    id(0:2): x;
   };
  };
 }
})"),
        std::make_pair("~((~x))", R"(
program {
 expression_list(0:0) {
  not(0:0) {
   parenthesized(0:1) {
    parenthesized(0:2) {
     not(0:3) {
      id(0:4): x;
     };
    };
   };
  };
 }
})"),
        std::make_pair("(~((y || x) && (~x)) || s0me_we1rd_th1ngy);", R"(
program {
 expression_list(0:0) {
  parenthesized(0:0) {
   or(0:1) {
    not(0:1) {
     parenthesized(0:2) {
      and(0:3) {
       parenthesized(0:3) {
        or(0:4) {
         id(0:4): y;
         id(0:9): x;
        };
       };
       parenthesized(0:15) {
        not(0:16) {
         id(0:17): x;
        };
       };
      };
     };
    };
    id(0:24): s0me_we1rd_th1ngy;
   };
  };
 }
})"),
        std::make_pair("(~y || x)", R"(
program {
 expression_list(0:0) {
  parenthesized(0:0) {
   or(0:1) {
    not(0:1) {
     id(0:2): y;
    };
    id(0:7): x;
   };
  };
 }
})")
    )
);
