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
 * \c array_view on \c uint8_t* and \c char*
 */
struct writable_bytes_view : writable_array_view<uint8_t>
{
    writable_bytes_view() = default;
    writable_bytes_view(writable_bytes_view &&) = default;
    writable_bytes_view(writable_bytes_view const &) = default;
    writable_bytes_view & operator=(writable_bytes_view &&) = default;
    writable_bytes_view & operator=(writable_bytes_view const &) = default;


    template<class T, std::size_t n>
    writable_bytes_view(T(&)[n]) = delete;

    template<class T, std::size_t n>
    writable_bytes_view & operator=(T(&)[n]) = delete;


    constexpr writable_bytes_view(std::nullptr_t) noexcept
    : writable_array_view(nullptr)
    {}

    constexpr writable_bytes_view(writable_byte_ptr const p, std::size_t sz) noexcept
    : writable_array_view(p.as_u8p(), sz)
    {}

    constexpr writable_bytes_view(writable_byte_ptr first, writable_byte_ptr last) noexcept
    : writable_array_view(first.as_u8p(), last.as_u8p())
    {}

    writable_bytes_view(writable_array_view<char> av) noexcept /*NOLINT*/
    : writable_array_view(byte_ptr_cast(av.data()), av.size())
    {}

    writable_bytes_view(writable_array_view<uint8_t> av) noexcept /*NOLINT*/
    : writable_array_view(av.data(), av.size())
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<writable_array_view<uint8_t>, U&&>::value, bool
    >::type = 1>
    explicit constexpr writable_bytes_view(U && a) noexcept /*NOLINT*/
    : writable_array_view(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<writable_array_view<char>, U&&>::value, bool
    >::type = 1>
    explicit constexpr writable_bytes_view(U && a) noexcept /*NOLINT*/
    : writable_bytes_view(writable_array_view<char>(a))
    {}


    [[nodiscard]] char       * as_charp() noexcept { return char_ptr_cast(this->data()); }
    [[nodiscard]] char const * as_charp() const noexcept { return char_ptr_cast(this->data()); }
    [[nodiscard]] constexpr uint8_t       * as_u8p() noexcept { return this->data(); }
    [[nodiscard]] constexpr uint8_t const * as_u8p() const noexcept { return this->data(); }

    [[nodiscard]] writable_array_view<char> as_chars() noexcept
    { return writable_array_view<char>{this->as_charp(), this->size()}; }
    [[nodiscard]]          array_view<char> as_chars() const noexcept { return {this->as_charp(), this->size()}; }
};

/**
 * \c array_view on constant \c uint8_t* and \c char*
 */
struct bytes_view : array_view<uint8_t>
{
    bytes_view() = default;
    bytes_view(bytes_view &&) = default;
    bytes_view(bytes_view const &) = default;
    bytes_view & operator=(bytes_view &&) = default;
    bytes_view & operator=(bytes_view const &) = default;


    template<class T, std::size_t n>
    bytes_view(T(&)[n]) = delete;

    template<class T, std::size_t n>
    bytes_view & operator=(T(&)[n]) = delete;


    constexpr bytes_view(std::nullptr_t) noexcept
    : array_view<uint8_t>(nullptr)
    {}

    constexpr bytes_view(byte_ptr p, std::size_t sz) noexcept
    : array_view<uint8_t>(p.as_u8p(), sz)
    {}

    constexpr bytes_view(byte_ptr first, byte_ptr last) noexcept
    : array_view<uint8_t>(first.as_u8p(), last.as_u8p())
    {}

    bytes_view(array_view<char> av) noexcept /*NOLINT*/
    : array_view<uint8_t>(byte_ptr_cast(av.data()), av.size())
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<uint8_t>, U&&>::value, bool
    >::type = 1>
    constexpr bytes_view(U && a) noexcept(noexcept(array_view<uint8_t>(a))) /*NOLINT*/
    : array_view<uint8_t>(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<char>, U&&>::value, bool
    >::type = 1>
    constexpr bytes_view(U && a) noexcept(noexcept(array_view<char>(a))) /*NOLINT*/
    : bytes_view(array_view<char>(a))
    {}


    [[nodiscard]] char const * as_charp() const noexcept { return char_ptr_cast(this->data()); }
    [[nodiscard]] constexpr uint8_t const * as_u8p() const noexcept { return this->data(); }

    [[nodiscard]] array_view<char> as_chars() const noexcept { return {this->as_charp(), this->size()}; }
};
