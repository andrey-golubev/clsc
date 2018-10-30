// Copyright 2018 Andrey Golubev
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

#ifndef _CLEAR_HPP_
#define _CLEAR_HPP_

#include <functional>
#include <type_traits>
#include <cstring>

/**
 * \file clear.hpp
 * \brief File defines simple utility to clear type-independent object.
 *
 * The utility allows to clear type-independent object based on the POD-ness of this object.
 * If the object is POD, it is safely cleaned by default cleaner unless user-defined cleaner is provided.
 * If the object is non-POD, cleaner must be provided by the user.
 * Note: If the object is a pointer, prior dereferencing is required in order to modify the underlying data and not the address of the pointer.
 * Note: As the utility accepts user-defined way to work with object, cleaning is not guaranteed to be safe unless the default cleaner is used.
 */

namespace clsc
{
    template<typename ObjT, bool = std::is_pod<ObjT>::value>
    struct __clear
    {
        static void execute(ObjT& o, std::function<void(ObjT&)> cleaner)
        {
            cleaner(o);
        }
    };

    template<typename ObjT>
    struct __clear<ObjT, true>
    {
        static void execute(ObjT& o, std::function<void(ObjT&)> cleaner = [](ObjT& o) { std::memset(&o, 0, sizeof(o)); })
        {
            cleaner(o);
        }
    };


    template<typename ObjT>
    static void clear(ObjT& o)
    {
        __clear<ObjT>::execute(o);
    }

    template<typename ObjT>
    static void clear(ObjT& o, std::function<void(ObjT&)> cleaner)
    {
        __clear<ObjT>::execute(o, cleaner);
    }
}

#endif /* _CLEAR_HPP_ */
