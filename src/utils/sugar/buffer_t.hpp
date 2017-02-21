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

#include "utils/sugar/bytes_t.hpp"

/**
 * \c array_view on \c uint8_t* and \c char*
 */
struct buffer_t : bytes_array
{
    buffer_t() = default;
    buffer_t(buffer_t const &) = default;

    using bytes_array::bytes_array;

    template<class T, std::size_t n>
    buffer_t(T (& a)[n]) noexcept
    : bytes_array(a, n)
    {}
};

/**
 * \c array_view on \c uint8_t* and \c char*
 */
struct const_buffer_t : const_bytes_array
{
    const_buffer_t() = default;
    const_buffer_t(const_buffer_t const &) = default;

    using const_bytes_array::const_bytes_array;

    template<class T, std::size_t n>
    const_buffer_t(T (& a)[n]) noexcept
    : const_bytes_array(a, n)
    {}
};
