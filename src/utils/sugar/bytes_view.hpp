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

#include "utils/sugar/byte_ptr.hpp"
#include "utils/sugar/array_view.hpp"

/**
 * \c array_view on \c uint8_t* and \c char*, but without ctor(T[n])/operator=(T[n])
 */
struct bytes_view : array_view<uint8_t>
{
    bytes_view() = default;
    bytes_view(bytes_view const &) = default;
    bytes_view & operator=(bytes_view const &) = default;


    template<class T, std::size_t n>
    bytes_view(T(&)[n]) = delete;

    template<class T, std::size_t n>
    bytes_view & operator=(T(&)[n]) = delete;


    constexpr bytes_view(std::nullptr_t) noexcept
    : array_view<uint8_t>(nullptr)
    {}

    constexpr bytes_view(byte_ptr const p, std::size_t sz) noexcept
    : array_view<uint8_t>(p.to_u8p(), sz)
    {}

    constexpr bytes_view(byte_ptr p, byte_ptr pright) noexcept
    : array_view<uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    bytes_view(array_view<char> av) noexcept /*NOLINT*/
    : array_view<uint8_t>(byte_ptr_cast(av.data()), av.size())
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<uint8_t>, U&&>::value, bool
    >::type = 1>
    constexpr bytes_view(U && a) noexcept /*NOLINT*/
    : array_view<uint8_t>(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<char>, U&&>::value, bool
    >::type = 1>
    constexpr bytes_view(U && a) noexcept /*NOLINT*/
    : bytes_view(array_view<char>(a))
    {}


    char       * to_charp() noexcept { return char_ptr_cast(this->data()); }
    char const * to_charp() const noexcept { return char_ptr_cast(this->data()); }
    constexpr uint8_t       * to_u8p() noexcept { return this->data(); }
    constexpr uint8_t const * to_u8p() const noexcept { return this->data(); }

    array_view_char       as_chars() noexcept { return {this->to_charp(), this->size()}; }
    array_view_const_char as_chars() const noexcept { return {this->to_charp(), this->size()}; }
};

/**
 * \c array_view on \c uint8_t* and \c char*, but without ctor(T[n])/operator=(T[n])
 */
struct const_bytes_view : array_view<const uint8_t>
{
    const_bytes_view() = default;
    const_bytes_view(const_bytes_view const &) = default;
    const_bytes_view & operator=(const_bytes_view const &) = default;


    template<class T, std::size_t n>
    const_bytes_view(T(&)[n]) = delete;

    template<class T, std::size_t n>
    const_bytes_view & operator=(T(&)[n]) = delete;


    constexpr const_bytes_view(std::nullptr_t) noexcept
    : array_view<const uint8_t>(nullptr)
    {}

    constexpr const_bytes_view(const_byte_ptr p, std::size_t sz) noexcept
    : array_view<const uint8_t>(p.to_u8p(), sz)
    {}

    constexpr const_bytes_view(const_byte_ptr p, const_byte_ptr pright) noexcept
    : array_view<const uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    const_bytes_view(array_view<const char> const av) noexcept /*NOLINT*/
    : array_view<const uint8_t>(byte_ptr_cast(av.data()), av.size())
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<const uint8_t>, U&&>::value, bool
    >::type = 1>
    constexpr const_bytes_view(U && a) noexcept /*NOLINT*/
    : array_view<const uint8_t>(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<const char>, U&&>::value, bool
    >::type = 1>
    constexpr const_bytes_view(U && a) noexcept /*NOLINT*/
    : const_bytes_view(array_view<const char>(a))
    {}


    char const * to_charp() const noexcept { return char_ptr_cast(this->data()); }
    constexpr uint8_t const * to_u8p() const noexcept { return this->data(); }

    array_view_const_char as_chars() const noexcept { return {this->to_charp(), this->size()}; }
};

using cbytes_view = const_bytes_view;
