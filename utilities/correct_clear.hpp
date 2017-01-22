#ifndef _CORRECT_CLEAR_H_
#define _CORRECT_CLEAR_H_

#include <functional>
#include <type_traits>

/**
 * \file correct_clear.hpp
 * \brief File defines simple utility to clear type-independent object.
 *
 * The utility allows to clear type-independent object based on the POD-ness of this object.
 * If the object is POD, it is safely cleaned by default cleaner unless user-defined cleaner is provided.
 * If the object is non-POD, cleaner must be provided by the user.
 * As the utility accepts user-defined way to work with object, cleaning is not guaranteed to be safe unless the default cleaner is used.
 * If the object is a pointer, prior dereferencing is required in order to modify the underlying data and not the address of the pointer.
 * If the object is an array, custom cleaner is required as no size is provided to the utility.
 */

namespace clsc
{
    template<typename ObjT, bool = std::is_pod<ObjT>::value>
    struct _clear
    {
        static void execute(ObjT& o, std::function<void(ObjT&)> cleaner)
        {
            cleaner(o);
        }
    };

    template<typename ObjT>
    struct _clear<ObjT, true>
    {
        static void execute(ObjT& o, std::function<void(ObjT&)> cleaner = [](ObjT& o) { o = {}; })
        {
            cleaner(o);
        }
    };




    template<typename ObjT>
    static void clear(ObjT& o)
    {
        _clear<ObjT, std::is_pod<ObjT>::value>::execute(o);
    }

    template<typename ObjT>
    static void clear(ObjT& o, std::function<void(ObjT&)> cleaner)
    {
        _clear<ObjT, std::is_pod<ObjT>::value>::execute(o, cleaner);
    }
}

#endif /* _CORRECT_CLEAR_H_ */
