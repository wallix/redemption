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
 * \c array_view on \c uint8_t*, \c char*, \c uint8_t[n], \c char[n]
 */
struct buffer_t : bytes_view
{
    buffer_t() = default;
    buffer_t(buffer_t const &) = default;

    buffer_t & operator=(buffer_t const &) = default;

    using bytes_view::bytes_view;

    template<class T, std::size_t n>
    buffer_t(T (& a)[n]) noexcept
    : bytes_view(a, n)
    {}

    template<class T>
    buffer_t(T&& a) noexcept(noexcept(bytes_view(static_cast<T&&>(a))))
    : bytes_view(static_cast<T&&>(a))
    {}
};

/**
 * \c const_array_view on \c uint8_t*, \c char*, \c uint8_t[n], \c char[n]
 */
struct const_buffer_t : const_bytes_view
{
    const_buffer_t() = default;
    const_buffer_t(const_buffer_t const &) = default;

    const_buffer_t & operator=(const_buffer_t const &) = default;

    using const_bytes_view::const_bytes_view;

    template<class T, std::size_t n>
    const_buffer_t(T (& a)[n]) noexcept
    : const_bytes_view(a, n)
    {}
};
