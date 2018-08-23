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

#include <cstddef>


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
