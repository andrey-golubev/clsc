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

#include <besc_lexer.hpp>

#include <gtest/gtest.h>

#include <sstream>

TEST(besc_lexer_tests, creatable) {
    std::stringstream ss;
    clsc::bes::lexer lexer{std::cin, ss};
    (void)lexer;
}

void tokenize(const std::pair<std::string, std::string>& in_out) {
    std::stringstream ssin;
    std::stringstream ssout;
    ssin << in_out.first;

    auto input = ssin.str();
    clsc::bes::lexer lexer(ssin, ssout);
    try {
        lexer.tokenize();
    } catch (const std::exception& e) {
        FAIL() << "Tokenizing \"" << input << "\" failed with: " << e.what() << std::endl;
    }

    EXPECT_EQ(in_out.second, ssout.str());
}

TEST(besc_lexer_tests, standalone_tokens) {
    std::vector<std::pair<std::string, std::string>> in_out_data{
        {"||", "OR 0:0\n"},
        {"&&", "AND 0:0\n"},
        {"~", "NOT 0:0\n"},
        {"^", "XOR 0:0\n"},
        {"->", "ARROW_RIGHT 0:0\n"},
        {"<-", "ARROW_LEFT 0:0\n"},
        {"==", "EQ 0:0\n"},
        {"!=", "NEQ 0:0\n"},
        {"=", "ASSIGN 0:0\n"},
        {"symbol", "ALIAS 0:0\n"},
        {"var", "VAR 0:0\n"},
        {"eval", "EVAL 0:0\n"},
        {";", "SEMICOLON 0:0\n"},
        {"x", "IDENTIFIER 0:0\n"},
        {"_90iyu", "IDENTIFIER 0:0\n"},
        {"true", "LITERAL_TRUE 0:0\n"},
        {"false", "LITERAL_FALSE 0:0\n"},
        {"\"\"", "LITERAL_STRING 0:0\n"},
        {"\"hello! world\"", "LITERAL_STRING 0:0\n"},
    };
    for (const auto& in_out : in_out_data) {
        tokenize(in_out);
    }
}

TEST(besc_lexer_tests, many_tokens) {
    std::vector<std::pair<std::string, std::string>> in_out_data{
        {"symbol x;", "ALIAS 0:0\nIDENTIFIER 0:7\nSEMICOLON 0:8\n"},
        {
            "_x == _01y",
            "IDENTIFIER 0:0\nEQ 0:3\nIDENTIFIER 0:6\n",
        },
        {"symbol x = \"foo && bar\";",
         "ALIAS 0:0\nIDENTIFIER 0:7\nASSIGN 0:9\nLITERAL_STRING 0:11\nSEMICOLON 0:23\n"},
    };
    for (const auto& in_out : in_out_data) {
        tokenize(in_out);
    }
}

TEST(besc_lexer_tests, DISABLED_many_tokens_extra) {
    std::vector<std::pair<std::string, std::string>> in_out_data{
        {"_x==_01y", "IDENTIFIER 0:0\nEQ 0:2\nIDENTIFIER 0:4\n"},
    };
    for (const auto& in_out : in_out_data) {
        tokenize(in_out);
    }
}
