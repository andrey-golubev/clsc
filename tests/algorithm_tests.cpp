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

#include <algorithm.hpp>

#include "common.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <limits>
#include <list>
#include <random>
#include <vector>

namespace {
template<typename T> bool are_equal(T a, T b, int units_in_last_place = 2) {
    return std::abs(a - b) <= std::numeric_limits<T>::epsilon() *
                                  std::max(std::abs(a), std::abs(b)) * units_in_last_place ||
           std::abs(a - b) < std::numeric_limits<T>::min();  // subnormal result
}

template<typename ValueType, typename DistributionType, typename GroupOperation,
         typename TestOperation>
void power_group_test_template(GroupOperation op, TestOperation test_op, int multiplier_max,
                               ValueType value_max, bool negate_multiplier = false,
                               bool negate_value = false) {
    std::array<int, 12> multipliers{};
    std::array<ValueType, 12> values{};

    std::mt19937 generator{};  // not using random device as seed to have reproducibility
    int multiplier_a, multiplier_b;
    std::tie(multiplier_a, multiplier_b) = negate_multiplier ? std::make_tuple(-multiplier_max, -2)
                                                             : std::make_tuple(2, multiplier_max);
    std::uniform_int_distribution<> multiplier_distribution(multiplier_a, multiplier_b);
    ValueType value_a, value_b;
    std::tie(value_a, value_b) = negate_value
                                     ? std::make_tuple(-ValueType(value_max), -ValueType(2))
                                     : std::make_tuple(ValueType(2), ValueType(value_max));
    DistributionType value_distribution(value_a, value_b);

    for (int i = 0; i < 10; ++i) {
        multipliers[i] = multiplier_distribution(generator);
        values[i] = value_distribution(generator);
    }

    // special cases:
    multipliers[10] = negate_multiplier ? -1 : 1;
    values[10] = negate_value ? -ValueType(1) : ValueType(1);
    multipliers[11] = 0;
    values[11] = ValueType(0);

    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            const auto multiplier = multipliers[i];
            const auto value = values[j];

            ValueType expected = test_op(value, multiplier);
            ValueType actual = clsc::power_group(value, multiplier, op);
            // first test with precision - important for doubles, if failed, go through gtest
            if (!are_equal(expected, actual, 6)) {
                EXPECT_EQ(expected, actual);
            }
        }
    }
}
}  // namespace

TEST(power_tests, multiply_integers) {
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::plus<int>{}, std::multiplies<int>{}, 100, 1000, false, false);
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::plus<int>{}, std::multiplies<int>{}, 100, 1000, true, false);
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::plus<int>{}, std::multiplies<int>{}, 100, 1000, false, true);
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::plus<int>{}, std::multiplies<int>{}, 100, 1000, true, true);
}
TEST(power_tests, multiply_doubles) {
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::plus<double>{}, std::multiplies<double>{}, 100, 1000, false, false);
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::plus<double>{}, std::multiplies<double>{}, 100, 1000, true, false);
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::plus<double>{}, std::multiplies<double>{}, 100, 1000, false, true);
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::plus<double>{}, std::multiplies<double>{}, 100, 1000, true, true);
}

TEST(power_tests, raise_integers_to_power) {
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::multiplies<int>{}, [](int v, int m) { return int(std::pow(v, m)); }, 5, 31, false,
        false);
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::multiplies<int>{}, [](int v, int m) { return int(std::pow(v, m)); }, 5, 31, true,
        false);
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::multiplies<int>{}, [](int v, int m) { return int(std::pow(v, m)); }, 5, 31, false,
        true);
    power_group_test_template<int, std::uniform_int_distribution<int>>(
        std::multiplies<int>{}, [](int v, int m) { return int(std::pow(v, m)); }, 5, 31, true,
        true);
}
TEST(power_tests, raise_doubles_to_power) {
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::multiplies<double>{}, [](double v, double m) { return std::pow(v, m); }, 5, 31.0,
        false, false);
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::multiplies<double>{}, [](double v, double m) { return std::pow(v, m); }, 3, 31.0, true,
        false);
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::multiplies<double>{}, [](double v, double m) { return std::pow(v, m); }, 5, 31.0,
        false, true);
    power_group_test_template<double, std::uniform_real_distribution<double>>(
        std::multiplies<double>{}, [](double v, double m) { return std::pow(v, m); }, 5, 31.0, true,
        true);
}

TEST(median_tests, edge_cases) {
    std::vector<int> x{0, 1, 2};

    {
        std::vector<int> medians(1, -1);
        clsc::experimental::median(x.begin(), x.end(), x.end(), x.end(), medians.begin());
        EXPECT_EQ(1, medians[0]);
    }

    {
        std::vector<int> medians(1, -1);
        clsc::experimental::median(x.end(), x.end(), x.begin(), x.end(), medians.begin());
        EXPECT_EQ(1, medians[0]);
    }

    {
        std::vector<int> medians(1, -1);
        clsc::experimental::median(x.end(), x.end(), x.end(), x.end(), medians.begin());
        EXPECT_EQ(-1, medians[0]);
    }
}

TEST(median_tests, equal_ranges) {
    {
        std::list<int> x{0, 1, 2};
        std::list<int> y{1, 7, 8};
        std::vector<int> medians = {-1, -1};
        clsc::experimental::median(x.begin(), x.end(), y.begin(), y.end(), medians.begin());
        EXPECT_EQ(1, medians[0]);
        EXPECT_EQ(2, medians[1]);

        medians = {-1, -1};
        clsc::experimental::median(y.begin(), y.end(), x.begin(), x.end(), medians.begin());
        EXPECT_EQ(1, medians[0]);
        EXPECT_EQ(2, medians[1]);
    }

    {
        std::vector<int> x{0, 1, 2, 3};
        std::vector<int> y{1, 3, 7, 8};
        std::vector<int> medians = {-1, -1};
        clsc::experimental::median(x.begin(), x.end(), y.begin(), y.end(), medians.begin());
        EXPECT_EQ(2, medians[0]);
        EXPECT_EQ(3, medians[1]);

        medians = {-1, -1};
        clsc::experimental::median(y.begin(), y.end(), x.begin(), x.end(), medians.begin());
        EXPECT_EQ(2, medians[0]);
        EXPECT_EQ(3, medians[1]);
    }
}

TEST(median_tests, unequal_ranges) {}
