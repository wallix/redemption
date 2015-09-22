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

#ifndef REDEMPTION_SRC_CONFIGS_CONFIGS_C_STR_BUF_HPP
#define REDEMPTION_SRC_CONFIGS_CONFIGS_C_STR_BUF_HPP

#include <limits>
#include <string>

namespace configs {

namespace detail {
    template<class T, bool = std::is_integral<T>::value, bool = std::is_enum<T>::value>
    struct TaggedCStrBuf;

    template<class T>
    struct TaggedCStrBuf<T, true, false>
    {
        static constexpr std::size_t size() {
            return std::numeric_limits<T>::digits10 + 2;
        }
        char buf[std::numeric_limits<T>::digits10 + 2];
        char * get() { return this->buf; }
    };

    template<class T>
    struct TaggedCStrBuf<T, false, true>
    : TaggedCStrBuf<typename std::underlying_type<T>::type, true, false>
    {};

    template<class T>
    struct TaggedCStrBuf<T, false, false>
    {};
}

template<class T>
struct CStrBuf : detail::TaggedCStrBuf<T>
{};

template<class T>
inline char const * c_str(CStrBuf<T>& , T const &);

template<class T>
CStrBuf<T> make_c_str_buf(T const & x) {
    CStrBuf<T> buf;
    c_str(buf, x);
    return buf;
}

}

#endif
