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

#include "tokens.hpp"

#include <ostream>

namespace clsc {
namespace bes {
struct token_stream {
    token_stream& get(annotated_token& t) {
        t = m_buf.front();
        m_buf.erase(m_buf.begin(), m_buf.begin() + 1);
        return *this;
    }

    annotated_token peek() const {
        if (!good()) {
            return annotated_token{};
        }
        return m_buf.front();
    }

    // TODO: lookahead(2)
    annotated_token peek2() const {
        if (m_buf.size() < 2) {
            return annotated_token{};
        }
        return m_buf[1];
    }

    bool good() const { return !m_buf.empty(); }

    friend token_stream& operator<<(token_stream& ts, annotated_token t) {
        ts.m_buf.push_back(std::move(t));
        return ts;
    }

    friend std::ostream& operator<<(std::ostream& os, const token_stream& ts) {
        for (const auto& t : ts.m_buf) {
            os << std::string(t) << '\n';
        }
        return os;
    }

private:
    std::vector<annotated_token> m_buf{};
};
}  // namespace bes
}  // namespace clsc
