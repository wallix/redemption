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

#ifndef REDEMPTION_GDI_UTILS_NON_NULL_HPP
#define REDEMPTION_GDI_UTILS_NON_NULL_HPP

#include <type_traits>
#include <cassert>

namespace gdi { namespace utils {

template<class T>
struct non_null
{
    static_assert(std::is_pointer<T>::value, "");

    non_null(decltype(nullptr)) = delete;
    non_null(int) = delete;
    non_null(T p)
    : p(p)
    { assert(p); }

    typename std::remove_pointer<T>::type & operator * () { return *p; }
    T operator->() { return p; }

private:
    T p;
};

} }

#endif
