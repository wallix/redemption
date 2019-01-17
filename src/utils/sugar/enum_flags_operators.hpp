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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "cxx/cxx.hpp"

namespace detail_
{
    template<bool>
    struct enable_if_impl
    {
        template<class T> using type = T;
    };

    template<>
    struct enable_if_impl<false>;
}  // namespace detail_

template<class T>
struct is_enum_flags : std::false_type
{};

template<class T>
using enable_if_enum_flags
  = typename detail_::enable_if_impl<is_enum_flags<T>::value>
  ::template type<T>;

template<class T>
constexpr
enable_if_enum_flags<T>
operator ~ (T x) noexcept
{ return T(~underlying_cast(x)); }

template<class T>
constexpr
enable_if_enum_flags<T>
operator & (T x, T y) noexcept
{ return T(underlying_cast(x) & underlying_cast(y)); }

template<class T>
constexpr
enable_if_enum_flags<T>
operator | (T x, T y) noexcept
{ return T(underlying_cast(x) | underlying_cast(y)); }

template<class T>
REDEMPTION_CXX14_CONSTEXPR
enable_if_enum_flags<T> &
operator &= (T & x, T y) noexcept
{ return x = T(underlying_cast(x) & underlying_cast(y)); }

template<class T>
REDEMPTION_CXX14_CONSTEXPR
enable_if_enum_flags<T> &
operator |= (T & x, T y) noexcept
{ return x = T(underlying_cast(x) | underlying_cast(y)); }
