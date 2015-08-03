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

template<class T>
struct CStrBuf {};

#define DECLARE_BUF(T)                                  \
    template<> struct CStrBuf<T> {                      \
        static constexpr std::size_t size() {           \
          return std::numeric_limits<T>::digits10 + 2;  \
        }                                               \
        char buf[std::numeric_limits<T>::digits10 + 2]; \
        char * get() { return this->buf; }        \
    }

DECLARE_BUF(int);
DECLARE_BUF(unsigned);

template<> struct CStrBuf<std::string> {};

}

#endif
