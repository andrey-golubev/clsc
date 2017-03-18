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
        bool operator!=(const ContainerType& rhs)
        {
            return !((*static_cast<ContainerType*>(this)) == rhs);
        }

        bool operator<=(const ContainerType& rhs)
        {
            return ((*static_cast<ContainerType*>(this)) < rhs || (*static_cast<ContainerType*>(this)) == rhs);
        }

        bool operator>(const ContainerType& rhs)
        {
            return !((*static_cast<ContainerType*>(this)) <= rhs);
        }

        bool operator>=(const ContainerType& rhs)
        {
            return !((*static_cast<ContainerType*>(this)) < rhs);
        }
    };



    struct __has_operator_base
    {
    protected:
        using yes = char;
        struct no { char _[2]; };
    };

    template<typename Type>
    struct __has_equal_to : public __has_operator_base
    {
    private:
        template<typename ContainerType, bool (ContainerType::*)(const ContainerType&) = (&ContainerType::operator==)>
        static yes __has_equal_to_operator(ContainerType*);
        static no  __has_equal_to_operator(...);

    public:
        static const bool value = sizeof(__has_equal_to_operator(static_cast<Type*>(0))) == sizeof(yes);
    };
    template<typename T> const bool __has_equal_to<T>::value;

    template<typename Type>
    struct __has_less_than : public __has_operator_base
    {
    private:
        template<typename ContainerType, bool (ContainerType::*)(const ContainerType&) = (&ContainerType::operator<)>
        static yes __has_less_than_operator(ContainerType*);
        static no  __has_less_than_operator(...);

    public:
        static const bool value = sizeof(__has_less_than_operator(static_cast<Type*>(0))) == sizeof(yes);
    };
    template<typename T> const bool __has_less_than<T>::value;

    template<typename ContainerType>
    struct adjustable_comparable
    {
        template<typename T = ContainerType>
        std::enable_if_t<__has_equal_to<T>::value, bool>
        operator!=(const T& rhs)
        {
            return !((*static_cast<ContainerType*>(this)) == rhs);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_equal_to<T>::value && __has_less_than<T>::value, bool>
        operator<=(const T& rhs)
        {
            return ((*static_cast<ContainerType*>(this)) < rhs || (*static_cast<ContainerType*>(this)) == rhs);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_equal_to<T>::value && __has_less_than<T>::value, bool>
        operator>(const T& rhs)
        {
            return !((*static_cast<ContainerType*>(this)) <= rhs);
        }

        template<typename T = ContainerType>
        std::enable_if_t<__has_less_than<T>::value, bool>
        operator>=(const T& rhs)
        {
            return !((*static_cast<ContainerType*>(this)) < rhs);
        }
    };
}

#endif /* _COMPARABLE_HPP_ */
