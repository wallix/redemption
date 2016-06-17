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

#ifndef REDEMPTION_UTILS_BYTES_T_HPP
#define REDEMPTION_UTILS_BYTES_T_HPP

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

private:
    uint8_t * data_ = nullptr;
};

struct const_bytes_t
{
    const_bytes_t() = default;

    const_bytes_t(char const * data) noexcept
    : data_(reinterpret_cast<uint8_t const *>(data))
    {}

    const_bytes_t(uint8_t const * data) noexcept
    : data_(data)
    {}

    const_bytes_t(bytes_t bytes) noexcept
    : data_(bytes)
    {}

    char const * to_charp() const noexcept { return reinterpret_cast<char const *>(this->data_); }
    uint8_t const * to_u8p() const noexcept { return this->data_; }

    operator char const * () const noexcept { return reinterpret_cast<char const *>(this->data_); }
    operator uint8_t const * () const noexcept { return this->data_; }

private:
    uint8_t const * data_ = nullptr;
};


struct bytes_array : array_view<uint8_t>
{
    bytes_array() = default;

    template<class T>
    bytes_array(T & v) noexcept
    : bytes_array(to_array(v))
    {}

    bytes_array(array_view<char> v) noexcept
    : array_view<uint8_t>({reinterpret_cast<uint8_t *>(v.data()), v.size()})
    {}

    bytes_array(array_view<uint8_t> v) noexcept
    : array_view<uint8_t>(v)
    {}

    bytes_array(bytes_t p, std::size_t sz)
    : array_view<uint8_t>(p.to_u8p(), sz)
    {}

    bytes_array(bytes_t p, bytes_t pright)
    : array_view<uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    bytes_array & operator=(bytes_array &) = default;

    template<class T, class = decltype(to_array(*static_cast<T*>(nullptr)))>
    bytes_array & operator=(T & other) {
        return operator=(to_array(other));
    }

    template<class T>
    bytes_array & operator=(array_view<char> & other) {
        static_cast<array_view<uint8_t>&>(*this) = {
           reinterpret_cast<uint8_t *>(other.data()), other.size()
        };
        return *this;
    }

    template<class T>
    bytes_array & operator=(array_view<uint8_t> & other) {
        static_cast<array_view<uint8_t>&>(*this) = other;
        return *this;
    }

    operator array_view<char> () noexcept {
        return {reinterpret_cast<char *>(this->data()), this->size()};
    }
    operator array_view<const char> () const noexcept {
        return {reinterpret_cast<char const *>(this->data()), this->size()};
    }
    operator array_view<const uint8_t> () const noexcept {
        return {this->data(), this->size()};
    }

private:
    template<class T>
    static auto to_array(T & v) noexcept -> decltype(array_view<char>(v)) {
        return array_view<char>(v);
    }

    template<class T>
    static auto to_array(T & v) noexcept -> decltype(array_view<uint8_t>(v)) {
        return array_view<uint8_t>(v);
    }
};

struct const_bytes_array : array_view<const uint8_t>
{
    const_bytes_array() = default;

    template<class T>
    const_bytes_array(T & v) noexcept
    : const_bytes_array(to_array(v))
    {}

    const_bytes_array(array_view<const char> v) noexcept
    : array_view<const uint8_t>({reinterpret_cast<uint8_t const *>(v.data()), v.size()})
    {}

    const_bytes_array(array_view<const uint8_t> v) noexcept
    : array_view<const uint8_t>(v)
    {}

    const_bytes_array(const_bytes_t p, std::size_t sz)
    : array_view<const uint8_t>(p.to_u8p(), sz)
    {}

    const_bytes_array(const_bytes_t p, const_bytes_t pright)
    : array_view<const uint8_t>(p.to_u8p(), pright.to_u8p())
    {}

    const_bytes_array & operator=(const_bytes_array const &) = default;

    template<class T, class = decltype(to_array(*static_cast<T*>(nullptr)))>
    const_bytes_array & operator=(T & other) {
        return operator=(to_array(other));
    }

    template<class T>
    const_bytes_array & operator=(array_view<const char> & other) {
        static_cast<array_view<const uint8_t>&>(*this) = {
           reinterpret_cast<uint8_t const *>(other.data()), other.size()
        };
        return *this;
    }

    template<class T>
    const_bytes_array & operator=(array_view<const uint8_t> & other) {
        static_cast<array_view<const uint8_t>&>(*this) = other;
        return *this;
    }

    operator array_view<const char> () const noexcept {
        return {reinterpret_cast<char const *>(this->data()), this->size()};
    }

private:
    template<class T>
    static auto to_array(T & v) noexcept -> decltype(array_view<const char>(v)) {
        return array_view<const char>(v);
    }

    template<class T>
    static auto to_array(T & v) noexcept -> decltype(array_view<const uint8_t>(v)) {
        return array_view<const uint8_t>(v);
    }
};

#endif
