// Copyright 2018-2019 Andrey Golubev
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

#include <comparable.hpp>
#include <gtest/gtest.h>

#include "common.hpp"

#define UNUSED(x) (void)x;

TEST(comparable_tests, simple_inheritance) {
  class A : public clsc::comparable<A> {
    int m_val = 0;

  public:
    A() = delete;
    A(const int &val) : m_val(val) {}

    int operator()(const A &rhs) { return m_val - rhs.m_val; }
  };

  A a(1), b(2);
  tests_common::compare(a, b);
}

TEST(comparable_tests, multiple_inheritance) {
  class operator_mock {
    int m_val = 0;

  public:
    operator_mock() = delete;
    operator_mock(int val) : m_val(val) {}
    int operator()(const operator_mock &rhs) { return m_val - rhs.m_val; }
  };

  class A : public clsc::comparable<A>, public operator_mock {
  public:
    A() = delete;
    A(int val) : operator_mock(val) {}
  };

  A a(1), b(2);
  tests_common::compare(a, b);
}

TEST(comparable_tests, multilevel_inheritance) {
  class operator_mock : public clsc::comparable<operator_mock> {
    int m_val = 0;

  public:
    operator_mock() = delete;
    operator_mock(int val) : m_val(val) {}
    int operator()(const operator_mock &rhs) { return m_val - rhs.m_val; }
  };

  class A : public operator_mock {
  public:
    A() = delete;
    A(int val) : operator_mock(val) {}
  };

  A a(1), b(2);
  tests_common::compare(a, b);
}

TEST(adjustable_comparable_tests, operator_exists) {
  class A : public clsc::adjustable_comparable<A> {
    int m_val = 0;

  public:
    A() = delete;
    A(const int &val) : m_val(val) {}

    int operator()(const A &rhs) { return m_val - rhs.m_val; }
  };

  A a(1), b(2);
  tests_common::compare(a, b);
}

TEST(adjustable_comparable_tests, no_operator) {
  class A : public clsc::adjustable_comparable<A> {
    int m_val = 0;

  public:
    A() = delete;
    A(const int &val) : m_val(val) { UNUSED(m_val); }
  };
  A a(1), b(2);
}
