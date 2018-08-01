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

#include "utils/sugar/cast.hpp"
#include "utils/sugar/array_view.hpp"

struct Uint32_le {
    uint8_t array[4];
    Uint32_le(uint16_t v){
        this->array[0] = static_cast<uint8_t>(v);
        this->array[1] = static_cast<uint8_t>(v >> 8);
        this->array[2] = static_cast<uint8_t>(v >> 16);
        this->array[3] = static_cast<uint8_t>(v << 24);
    }
};

struct Uint16_le {
    uint8_t array[2];
    Uint16_le(uint16_t v){
        this->array[0] = static_cast<uint8_t>(v);
        this->array[1] = static_cast<uint8_t>(v >> 8);
    }
};


struct byte_ptr
{
    byte_ptr() = default;

    byte_ptr(char * data) noexcept
    : data_(byte_ptr_cast(data))
    {}

    constexpr byte_ptr(uint8_t * data) noexcept
    : data_(data)
    {}

    char * to_charp() const noexcept { return char_ptr_cast(this->data_); }
    constexpr uint8_t * to_u8p() const noexcept { return this->data_; }

    operator char * () const noexcept { return to_charp(); }
    constexpr operator uint8_t * () const noexcept { return this->data_; }

    explicit operator bool () const noexcept { return this->data_; }

    uint8_t & operator[](std::size_t i) noexcept { return this->data_[i]; }
    constexpr uint8_t const & operator[](std::size_t i) const noexcept { return this->data_[i]; }

private:
    uint8_t * data_ = nullptr;
};

struct const_byte_ptr
{
    constexpr const_byte_ptr() = default;

    const_byte_ptr(char const * data) noexcept
    : data_(byte_ptr_cast(data))
    {}

    constexpr const_byte_ptr(uint8_t const * data) noexcept
    : data_(data)
    {}

    constexpr const_byte_ptr(byte_ptr bytes) noexcept
    : data_(bytes)
    {}

    char const * to_charp() const noexcept { return char_ptr_cast(this->data_); }
    constexpr uint8_t const * to_u8p() const noexcept { return this->data_; }

    operator char const * () const noexcept { return to_charp(); }
    constexpr operator uint8_t const * () const noexcept { return this->data_; }

    constexpr explicit operator bool () const noexcept { return this->data_; }

    constexpr uint8_t const & operator[](std::size_t i) const noexcept { return this->data_[i]; }

private:
    uint8_t const * data_ = nullptr;
};

using cbyte_ptr = const_byte_ptr;

/**
 * \c array_view on \c uint8_t* and \c char*, but without ctor(T[n])/operator=(T[n])
 */
struct byte_array : array_view<uint8_t>
{
    byte_array() = default;
    byte_array(byte_array const &) = default;
    byte_array & operator=(byte_array const &) = default;


    template<class T, std::size_t n>
    byte_array(T(&)[n]) = delete;

    template<class T, std::size_t n>
    byte_array & operator=(T(&)[n]) = delete;


    constexpr byte_array(std::nullptr_t) noexcept
    : array_view<uint8_t>(nullptr)
    {}

    constexpr byte_array(byte_ptr const p, std::size_t sz) noexcept
    : array_view<uint8_t>(p.to_u8p(), sz)
    {}

    constexpr byte_array(byte_ptr p, byte_ptr pright) noexcept
    : array_view<uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    byte_array(array_view<char> av) noexcept
    : array_view<uint8_t>(byte_ptr_cast(av.data()), av.size())
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<uint8_t>, U&&>::value, bool
    >::type = 1>
    constexpr byte_array(U && a) noexcept /*NOLINT*/
    : array_view<uint8_t>(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<char>, U&&>::value, bool
    >::type = 1>
    constexpr byte_array(U && a) noexcept /*NOLINT*/
    : byte_array(array_view<char>(a))
    {}


    char       * to_charp() noexcept { return char_ptr_cast(this->data()); }
    char const * to_charp() const noexcept { return char_ptr_cast(this->data()); }
    uint8_t       * to_u8p() noexcept { return this->data(); }
    constexpr uint8_t const * to_u8p() const noexcept { return this->data(); }
};

/**
 * \c array_view on \c uint8_t* and \c char*, but without ctor(T[n])/operator=(T[n])
 */
struct const_byte_array : array_view<const uint8_t>
{
    const_byte_array() = default;
    const_byte_array(const_byte_array const &) = default;
    const_byte_array & operator=(const_byte_array const &) = default;


    template<class T, std::size_t n>
    const_byte_array(T(&)[n]) = delete;

    template<class T, std::size_t n>
    const_byte_array & operator=(T(&)[n]) = delete;


    constexpr const_byte_array(std::nullptr_t) noexcept
    : array_view<const uint8_t>(nullptr)
    {}

    constexpr const_byte_array(const_byte_ptr p, std::size_t sz) noexcept
    : array_view<const uint8_t>(p.to_u8p(), sz)
    {}

    constexpr const_byte_array(const_byte_ptr p, const_byte_ptr pright) noexcept
    : array_view<const uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    const_byte_array(array_view<const char> const av) noexcept
    : array_view<const uint8_t>(byte_ptr_cast(av.data()), av.size())
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<const uint8_t>, U&&>::value, bool
    >::type = 1>
    constexpr const_byte_array(U && a) noexcept /*NOLINT*/
    : array_view<const uint8_t>(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<array_view<const char>, U&&>::value, bool
    >::type = 1>
    constexpr const_byte_array(U && a) noexcept /*NOLINT*/
    : const_byte_array(array_view<const char>(a))
    {}


    char const * to_charp() const noexcept { return char_ptr_cast(this->data()); }
    constexpr uint8_t const * to_u8p() const noexcept { return this->data(); }
};

using cbyte_array = const_byte_array;
