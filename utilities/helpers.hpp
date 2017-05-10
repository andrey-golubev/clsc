#ifndef _HELPERS_HPP_
#define _HELPERS_HPP_

#include <type_traits>
#include <string>
#include <vector>
#include <sstream>

namespace clsc
{
    namespace helpers
    {
        template<typename CharT, typename Traits = std::char_traits<CharT>>
        std::vector<std::basic_string<CharT>> split(const std::basic_string<CharT>& src, CharT delimiter)
        {
            std::vector<std::basic_string<CharT>> dst;
            std::basic_istringstream<CharT> ss_src(src);
            std::basic_string<CharT> tmp;
            while(std::getline(ss_src, tmp, delimiter))
            {
                dst.push_back(tmp);
            }
            return dst;
        }
    }
}

#endif // _HELPERS_HPP_
