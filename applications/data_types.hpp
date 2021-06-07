// Copyright 2021 Andrey Golubev
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

#include <vector>

namespace clsc {
namespace applications {

template<typename T> class Matrix {
    std::vector<std::vector<T>> m_data;

public:
    Matrix() = default;
    Matrix(size_t n, size_t m) : m_data(n, std::vector<T>(m)) {}
    Matrix(const std::vector<std::vector<T>>& d) : m_data(d) {}
    Matrix(std::vector<std::vector<T>>&& d) : m_data(std::move(d)) {}
    Matrix(std::initializer_list<std::vector<T>>&& list) : m_data(std::move(list)) {}

    std::vector<std::vector<T>>& data() { return m_data; }
    const std::vector<std::vector<T>>& data() const { return m_data; }

    friend bool operator==(const Matrix& x, const Matrix& y) {
        if (x.m_data.size() != y.m_data.size())
            return false;
        if (x.m_data.size() != 0 && x.m_data[0].size() != y.m_data[0].size())
            return false;

        bool equal = true;
        for (size_t i = 0; i < x.m_data.size(); ++i) {
            if (x.m_data[i] != y.m_data[i])
                return false;
        }
        return true;
    }
    friend bool operator!=(const Matrix& x, const Matrix& y) { return !(x == y); }
};

template<typename T, typename Plus, typename Times>
// Type requirements:
//  Plus - is associative and commutative semiring operation
//  Times - is associative semiring operation
//  Times (*) distributes over Plus (+), so that:
//      a * (b + c) = a * b + a * c
//      (b + c) * a = b * a + c * a
struct multiply_accumulate_matrix {
    Matrix<T> operator()(const Matrix<T>& x, const Matrix<T>& y) {
        assert(x.data().size() > 0);
        assert(y.data().size() > 0);
        assert(x.data()[0].size() == y.data().size());

        Plus add{};
        Times multiply{};

        Matrix<T> r(x.data().size(), y.data()[0].size());
        for (size_t i = 0; i < x.data().size(); ++i) {
            for (size_t j = 0; j < y.data().size(); ++j) {
                T sum{};
                for (size_t k = 0; k < y.data().size(); ++k) {
                    sum = add(sum, multiply(x.data()[i][k], y.data()[k][j]));
                }
                r.data()[i][j] = sum;
            }
        }
        return r;
    }
};

}  // namespace applications
}  // namespace clsc
