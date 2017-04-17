#ifndef _COMPARABLE_HPP_
#define _COMPARABLE_HPP_

/**
 * \file comparable.hpp
 * \brief File defines utility that provides functionality to compare two entities of the same user-defined type (e.g. two objects).
 *
 * The utility allows to compare two entities of the same type (basically, two objects of user-defined type)
 * providing a convenient built-in set of comparison operators.
 * To enable comparison operators one must provide operator() with the following (not strict) signature:
 * <integer_type/floating-point_type> operator()(T rhs) whereas [T = user-defined type].
 * The return value of such operator() should follow the rules:
 *    1. If A < B, then operator() < 0 whereas [A, B - some abstract entities].
 *    2. If A == B, then operator() == 0.
 *    3. If A > B, then operator() > 0.
 *    For instance, if A and B are compared via character strings one can use strcmp(A.str(), B.str()) as a return statement for operator().
 * Note: Current implementation supports multiple and multilevel inheritance. This might change in future revisions.
 * Note: Based on CRTP idiom.
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
