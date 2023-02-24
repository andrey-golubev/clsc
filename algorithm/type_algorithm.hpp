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

/**
 * \file type_algorithm.hpp
 * \brief File defines a number of algorithms. Currently a file for "everything".
 */
namespace clsc {
/*! \brief \c any_of<...>::value evaluates to \c true if at least one of the
 *         types in a type sequence satisfies \a UnaryPred predicate.
 */
template<template<typename U> typename UnaryPred, typename T, typename... Ts> struct any_of {
    static constexpr bool value = UnaryPred<T>::value || any_of<UnaryPred, Ts...>::value;
};

template<template<typename U> typename UnaryPred, typename T> struct any_of<UnaryPred, T> {
    static constexpr bool value = UnaryPred<T>::value;
};

/*! \brief \c all_of<...>::value evaluates to \c true if all of the types in a
 *         type sequence satisfy \a UnaryPred predicate.
 */
template<template<typename U> typename UnaryPred, typename T, typename... Ts> struct all_of {
    static constexpr bool value = UnaryPred<T>::value && all_of<UnaryPred, Ts...>::value;
};

template<template<typename U> typename UnaryPred, typename T> struct all_of<UnaryPred, T> {
    static constexpr bool value = UnaryPred<T>::value;
};

/*! \brief \c none_of<>::value evaluates to \c true if no type in a type
 *         sequence satisfies \a UnaryPred predicate.
 */
template<template<typename U> typename UnaryPred, typename T, typename... Ts> struct none_of {
    static constexpr bool value = !any_of<UnaryPred, T, Ts...>::value;
};

/*! \brief Converts \a Pred predicate into a unary predicate by binding first
           \c{N - 1} types of \a Us to \a Ts.
*/
template<template<typename... Us> typename Pred, typename... Ts> struct bind_to_unary {
    template<typename T> using type = Pred<Ts..., T>;
};
}  // namespace clsc
