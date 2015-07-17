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
#include "underlying_cast.hpp"

#define MK_ENUM_IO(E)                                    \
    template<class Ch, class Tr>                         \
    std::basic_ostream<Ch, Tr> &                         \
    operator << (std::basic_ostream<Ch, Tr> & os, E e) { \
        return os << underlying_cast(e);                 \
    }


#define MK_ENUM_FLAG_FN(E)                                                                             \
    MK_ENUM_IO(E)                                                                                      \
    inline E operator | (E x, E y) { return static_cast<E>(underlying_cast(x) | underlying_cast(y)); } \
    inline E operator & (E x, E y) { return static_cast<E>(underlying_cast(x) & underlying_cast(y)); } \
    inline E & operator |= (E & x, E y) { return x = x | y; }                                          \
    inline E & operator &= (E & x, E y) { return x = x | y; }

#endif
