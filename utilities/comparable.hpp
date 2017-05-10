#ifndef _COMPARABLE_HPP_
#define _COMPARABLE_HPP_

#include <type_traits>

/**
 * \file comparable.hpp
 * \brief File defines utility that provides functionality to compare two entities of the same user-defined type (e.g. two objects).
 *
 * The utility allows to compare two entities of the same type (basically, two objects of user-defined type)
 * providing a convenient built-in set of comparison operators.
 * To enable comparison operators one must provide `operator()` with the following (not strict) signature:
 * <integer_type/floating-point_type> operator()(T rhs) whereas [T = user-defined type].
 * The return value of such `operator()` should follow the rules:
 *    1. If A < B, then `operator()` < 0 whereas [A, B - some abstract entities].
 *    2. If A == B, then `operator()` == 0.
 *    3. If A > B, then `operator()` > 0.
 *    For instance, if A and B are compared via character strings one can use strcmp(A.str(), B.str()) as a return statement for `operator()`.
 * Note: Current implementation supports multiple and multilevel inheritance. This might change in future revisions.
 * Note: Based on CRTP idiom.
 */

namespace clsc
{
    /**
     * \class comparable
     * \brief Basic version of comparable class based on assumption that derived class has `operator()`.
     */
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



    struct __has_operator_base
    {
    protected:
        using yes = char;
        struct no { char _[2]; };
    };

    template<typename Type>
    struct __has_operator : public __has_operator_base
    {
    private:
        template<typename ContainerType, int (ContainerType::*)(const ContainerType&) = (&ContainerType::operator())>
        static yes m_has_operator(ContainerType*);
        static no  m_has_operator(...);

    public:
        static const bool value = sizeof(m_has_operator(static_cast<Type*>(nullptr))) == sizeof(yes);
    };
    template<typename T> const bool __has_operator<T>::value;

    /**
     * \class adjustable_comparable
     * \brief An adjustable version of comparable class based on existence of `operator()` in derived class.
     *
     * This version of comparable class has the same implementation.
     * However, it provides comparison operators only if derived class has correct `operator()`,
     * the class may be considered "empty" otherwise.
     * Note: though the implementation remains the same, all the comparison operators are templated
     * which means that inheritance (deeper/more advanced than `class A : public adjustable_comparable<A>`) is not supported.
     */
    template<typename ContainerType>
    struct adjustable_comparable
    {
        template<typename T = ContainerType>
        std::enable_if_t<__has_operator<T>::value, bool>
        operator==(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) == 0);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_operator<T>::value, bool>
        operator!=(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) != 0);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_operator<T>::value, bool>
        operator<(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) < 0);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_operator<T>::value, bool>
        operator<=(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) <= 0);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_operator<T>::value, bool>
        operator>(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) > 0);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_operator<T>::value, bool>
        operator>=(const ContainerType& rhs)
        {
            return static_cast<bool>((*static_cast<ContainerType*>(this))(rhs) >= 0);
        }
    };
}

#endif /* _COMPARABLE_HPP_ */
