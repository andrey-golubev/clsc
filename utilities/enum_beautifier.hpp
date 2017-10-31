#ifndef _ENUM_BEAUTIFIER_HPP_
#define _ENUM_BEAUTIFIER_HPP_

#include <iterator>
#include <type_traits>
#include <limits>

namespace clsc
{
    template <typename T, T first = std::numeric_limits<T>::min(), T last = std::numeric_limits<T>::max()>
    struct __for
    {
        template <typename Callable>
        constexpr T operator()(Callable&& f) const
        {
            if (first < last)
            {
                f(first);
                __for<T, first+static_cast<T>(1), last>()(f);
            }
        }
    };

    template <typename T, T n>
    struct __for<T, n, n>
    {
        template <typename Callable>
        constexpr T operator()(Callable&&) const
        {}
    };

    // TODO: how to determine that value is actually an existing enum type?
    template<typename EnumType, EnumType begin, EnumType end>
    struct __enum_values_detector
    {
        using utype = std::underlying_type_t<EnumType>;
        EnumType b = static_cast<EnumType>(__for<utype, begin, end>()([](auto value) { return value; }));
    };

    template<typename EnumType, EnumType begin, EnumType end>
    struct enum_beautifier :
//            public std::iterator<std::random_access_iterator_tag, EnumType>,
            public __enum_values_detector<EnumType, begin, end>
    {
    };
}

#endif /* _ENUM_BEAUTIFIER_HPP_ */
