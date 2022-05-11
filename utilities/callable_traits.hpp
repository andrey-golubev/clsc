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

#ifndef _CALLABLE_TRAITS_HPP_
#define _CALLABLE_TRAITS_HPP_

#include <tuple>
#include <type_traits>

#include <cstdint>

namespace clsc {

template<typename Signature>
struct callable_traits;

template<typename R, typename... Args>
struct callable_traits<R(Args...)>
{
    using return_type = R;
    constexpr static std::size_t args_size = sizeof... (Args);
    using args = std::tuple<Args...>;
    template<std::size_t Index>
    using arg_type = typename std::tuple_element<Index, args>::type;

    template<typename Callable>
    callable_traits(Callable &&c) {}
};

template<typename G, typename R, typename... Args>
struct callable_traits<R(G::*)(Args...)>
{
    using return_type = R;
    constexpr static std::size_t args_size = sizeof... (Args);
    using args = std::tuple<Args...>;
    template<std::size_t Index>
    using arg_type = typename std::tuple_element<Index, args>::type;

    template<typename Callable>
    callable_traits(Callable &&c) {}
};

template<typename Callable>
callable_traits(Callable) -> callable_traits<&Callable::operator()>;

}

#endif // _CALLABLE_TRAITS_HPP_
