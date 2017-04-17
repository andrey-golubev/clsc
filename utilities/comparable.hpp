#ifndef _COMPARABLE_HPP_
#define _COMPARABLE_HPP_

#include <type_traits>

/**
 * \file comparable.hpp
 * \brief File defines utility that provides functionality to compare two entities of the same user-defined type (e.g. two objects).
 *
 * Based on CRTP idiom.
 */

namespace clsc
{
    template<typename ContainerType>
    struct comparable
    {
        bool operator==(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) == 0);
        }

        bool operator!=(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) != 0);
        }

        bool operator<(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) < 0);
        }

        bool operator<=(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) <= 0);
        }

        bool operator>(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) > 0);
        }

        bool operator>=(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) >= 0);
        }
    };
}

#endif /* _COMPARABLE_HPP_ */
