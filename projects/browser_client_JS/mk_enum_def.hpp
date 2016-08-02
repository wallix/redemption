/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef MK_ENUM_IO

#include <iosfwd>
#include <cstdio>
#include <cstdlib>
#include "utils/sugar/underlying_cast.hpp"
#include "configs/c_str_buf.hpp"
#include <initializer_list>

#define MK_ENUM_IO(E)                                    \
    template<class Ch, class Tr>                         \
    std::basic_ostream<Ch, Tr> &                         \
    operator << (std::basic_ostream<Ch, Tr> & os, E e) { \
        return os << underlying_cast(e);                 \
    }                                                    \
                                                         \
    inline char const * get_enum_name(E) { return #E; }


#define MK_ENUM_FLAG_FN(E)                                                                             \
    MK_ENUM_IO(E)                                                                                      \
    inline E operator | (E x, E y) { return static_cast<E>(underlying_cast(x) | underlying_cast(y)); } \
    inline E operator & (E x, E y) { return static_cast<E>(underlying_cast(x) & underlying_cast(y)); } \
    inline E operator ~ (E x) { return static_cast<E>(~underlying_cast(x)) & E::FULL; }                \
    inline E & operator |= (E & x, E y) { return x = x | y; }                                          \
    inline E & operator &= (E & x, E y) { return x = x & y; }

#define MK_PARSER_ENUM_FLAGS(Enum)                                \
    MK_ENUM_FLAG_FN(Enum)                                         \
    inline void parse(Enum & e, char const * cstr) {              \
        char * end = nullptr;                                     \
        errno = 0;                                                \
        auto n = std::strtoul(cstr, &end, 10);                    \
        if (!errno && end && !*end                                \
         && n <= static_cast<unsigned long>(Enum::FULL)           \
        ) {                                                       \
            e = static_cast<Enum>(n);                             \
        }                                                         \
    }                                                             \
    inline int copy_val(Enum x, char * buff, std::size_t n) {     \
        return snprintf(buff, n, "%u", static_cast<unsigned>(x)); \
    }                                                             \
    template<> struct CStrBuf<Enum> : CStrBuf<unsigned> {};       \
    inline char const * c_str(CStrBuf<Enum>& s, Enum x) {         \
        copy_val(x, s.get(), s.size());                           \
        return s.get();                                           \
    }

#endif

#ifdef IN_IDE_PARSER
# define ENUM_OPTION(Enum, X, ...)
#else
# define ENUM_OPTION(Enum, X, ...)                       \
    template<class T> struct enum_option<Enum, T> {      \
        static constexpr const std::initializer_list<    \
            std::decay<decltype(X)>::type                \
        > value {X, __VA_ARGS__};                        \
        using type = std::true_type;                     \
    };                                                   \
    template<class T> constexpr const                    \
    std::initializer_list<std::decay<decltype(X)>::type> \
    enum_option<Enum, T>::value
#endif

#define MK_ENUM_FIELD(Enum, ...)                               \
    MK_ENUM_IO(Enum)                                           \
    ENUM_OPTION(Enum, __VA_ARGS__);                            \
    inline void parse(Enum & e, char const * cstr)  {          \
        unsigned i = 0;                                        \
        auto l = {__VA_ARGS__};                                \
        for (auto s : l) {                                     \
            if (0 == strcmp(cstr, s)) {                        \
                e = static_cast<Enum>(i);                      \
                break;                                         \
            }                                                  \
            ++i;                                               \
        }                                                      \
    }                                                          \
    inline int copy_val(Enum e, char * buff, std::size_t n)  { \
        char const * cstr;                                     \
        auto l = {__VA_ARGS__};                                \
        if (underlying_cast(e) >= underlying_cast(Enum::NB)) { \
            cstr = *l.begin();                                 \
        }                                                      \
        cstr = *(l.begin() + underlying_cast(e));              \
        return snprintf(buff, n, "%s", cstr);                  \
    }                                                          \
    inline char const * c_str(CStrBuf<Enum>& , Enum e) {       \
        auto l = {__VA_ARGS__};                                \
        if (underlying_cast(e) >= underlying_cast(Enum::NB)) { \
            return *l.begin();                                 \
        }                                                      \
        return *(l.begin() + underlying_cast(e));              \
    }

#ifndef REDEMPTION_SRC_CONFIGS_MK_ENUM_DEF_HPP
#define REDEMPTION_SRC_CONFIGS_MK_ENUM_DEF_HPP

namespace configs
{
    template<class E, class = void>
    struct enum_option
    { using type = std::false_type; };

    template<class E>
    decltype(*enum_option<E>::value.begin())
    enum_to_option(E e) {
        return *(enum_option<E>::value.begin() + underlying_cast(e));
    }

    template<class T>
    char const * get_enum_name(T) = delete;
}

#endif
