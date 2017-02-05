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
 * As the utility accepts user-defined way to work with object, cleaning is not guaranteed to be safe unless the default cleaner is used.
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
