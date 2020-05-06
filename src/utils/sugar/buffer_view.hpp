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

#include "utils/sugar/bytes_view.hpp"

/**
 * \c writable_bytes_view on \c uint8_t*, \c char*, \c uint8_t[n], \c char[n]
 */
struct writable_buffer_view : writable_bytes_view
{
    writable_buffer_view() = default;
    writable_buffer_view(writable_buffer_view const &) = default;

    writable_buffer_view & operator=(writable_buffer_view const &) = default;

    using writable_bytes_view::writable_bytes_view;

    writable_buffer_view(writable_bytes_view a) noexcept
    : writable_bytes_view(a)
    {}

    template<class T, std::size_t n>
    writable_buffer_view(T (& a)[n]) noexcept
    : writable_bytes_view(a, n)
    {}

    template<class T>
    writable_buffer_view(T&& a) noexcept(noexcept(writable_bytes_view(static_cast<T&&>(a))))
    : writable_bytes_view(static_cast<T&&>(a))
    {}
};

/**
 * \c bytes_view on \c uint8_t*, \c char*, \c uint8_t[n], \c char[n]
 */
struct buffer_view : bytes_view
{
    buffer_view() = default;
    buffer_view(buffer_view const &) = default;

    buffer_view & operator=(buffer_view const &) = default;

    using bytes_view::bytes_view;

    buffer_view(bytes_view a) noexcept
    : bytes_view(a)
    {}

    template<class T, std::size_t n>
    buffer_view(T (& a)[n]) noexcept
    : bytes_view(a, n)
    {}
};
