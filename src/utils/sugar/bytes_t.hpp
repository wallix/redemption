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

#include "utils/sugar/array_view.hpp"


struct bytes_t
{
    bytes_t() = default;

    bytes_t(char * data) noexcept
    : data_(reinterpret_cast<uint8_t*>(data))
    {}

    bytes_t(uint8_t * data) noexcept
    : data_(data)
    {}

    char * to_charp() const noexcept { return reinterpret_cast<char *>(this->data_); }
    uint8_t * to_u8p() const noexcept { return this->data_; }

    operator char * () const noexcept { return reinterpret_cast<char *>(this->data_); }
    operator uint8_t * () const noexcept { return this->data_; }

    explicit operator bool () const noexcept { return this->data_; }

    uint8_t & operator[](std::size_t i) noexcept { return this->data_[i]; }
    uint8_t const & operator[](std::size_t i) const noexcept { return this->data_[i]; }

private:
    uint8_t * data_ = nullptr;
};

struct const_bytes_t
{
    constexpr const_bytes_t() = default;

    const_bytes_t(char const * data) noexcept
    : data_(reinterpret_cast<uint8_t const *>(data))
    {}

    constexpr const_bytes_t(uint8_t const * data) noexcept
    : data_(data)
    {}

    const_bytes_t(bytes_t bytes) noexcept
    : data_(bytes)
    {}

    char const * to_charp() const noexcept { return reinterpret_cast<char const *>(this->data_); }
    constexpr uint8_t const * to_u8p() const noexcept { return this->data_; }

    operator char const * () const noexcept { return reinterpret_cast<char const *>(this->data_); }
    constexpr operator uint8_t const * () const noexcept { return this->data_; }

    constexpr explicit operator bool () const noexcept { return this->data_; }

    uint8_t const & operator[](std::size_t i) const noexcept { return this->data_[i]; }

private:
    uint8_t const * data_ = nullptr;
};


struct bytes_array : array_view<uint8_t>
{
    bytes_array() = default;
    bytes_array(bytes_array const &) = default;

    bytes_array(array_view<char> v) noexcept
    : array_view<uint8_t>({reinterpret_cast<uint8_t *>(v.data()), v.size()})
    {}

    bytes_array(array_view<uint8_t> v) noexcept
    : array_view<uint8_t>(v)
    {}

    template<class T>
    bytes_array(T & v) noexcept
    : bytes_array(make_array_view(v))
    {}

    bytes_array(bytes_t p, std::size_t sz)
    : array_view<uint8_t>(p.to_u8p(), sz)
    {}

    bytes_array(bytes_t p, bytes_t pright)
    : array_view<uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    bytes_array & operator=(bytes_array const &) = default;

    bytes_array & operator=(array_view<char> other) {
        static_cast<array_view<uint8_t>&>(*this) = {
           reinterpret_cast<uint8_t *>(other.data()), other.size()
        };
        return *this;
    }

    bytes_array & operator=(array_view<uint8_t> other) {
        static_cast<array_view<uint8_t>&>(*this) = other;
        return *this;
    }

    template<class T>
    bytes_array & operator=(T & other) noexcept
    { return (*this = make_array_view(other)); }
};

struct const_bytes_array : array_view<const uint8_t>
{
    const_bytes_array() = default;
    const_bytes_array(const_bytes_array const &) = default;

    const_bytes_array(array_view<const char> v) noexcept
    : array_view<const uint8_t>({reinterpret_cast<uint8_t const *>(v.data()), v.size()})
    {}

    constexpr const_bytes_array(array_view<const uint8_t> v) noexcept
    : array_view<const uint8_t>(v)
    {}

    constexpr const_bytes_array(const_bytes_t p, std::size_t sz)
    : array_view<const uint8_t>(p.to_u8p(), sz)
    {}

    constexpr const_bytes_array(const_bytes_t p, const_bytes_t pright)
    : array_view<const uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    constexpr const_bytes_array(bytes_array const & barray) noexcept
    : array_view<const uint8_t>(barray.data(), barray.size())
    {}

    template<class T>
    constexpr const_bytes_array(T & v) noexcept
    : const_bytes_array(make_array_view(v))
    {}

    const_bytes_array & operator=(const_bytes_array const &) = default;

    const_bytes_array & operator=(array_view<const char> other) {
        static_cast<array_view<const uint8_t>&>(*this) = {
           reinterpret_cast<uint8_t const *>(other.data()), other.size()
        };
        return *this;
    }

    const_bytes_array & operator=(array_view<const uint8_t> other) {
        static_cast<array_view<const uint8_t>&>(*this) = other;
        return *this;
    }

    const_bytes_array & operator=(bytes_array const & barray) noexcept
    { return (*this = static_cast<array_view<uint8_t> const &>(barray)); }

    template<class T>
    const_bytes_array & operator=(T & other) noexcept
    { return (*this = make_array_view(other)); }
};
