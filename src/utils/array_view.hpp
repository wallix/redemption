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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_UTILS_ARRAY_VIEW_HPP
#define REDEMPTION_UTILS_ARRAY_VIEW_HPP

#include <cstddef>

template<class T>
struct array_view
{
    using type = T;

    constexpr array_view() = default;

    constexpr array_view(std::nullptr_t)
    : array_view(nullptr, 0)
    {}

    constexpr array_view(type * p, std::size_t sz)
    : p(p)
    , sz(sz)
    {}

    template<class Cont>
    constexpr array_view(Cont & cont)
    : array_view(cont.data(), cont.size())
    {}

    template<std::size_t N>
    constexpr array_view(type (&a)[N])
    : array_view(a, N)
    {}

    constexpr explicit operator bool () const noexcept { return this->p; }

    constexpr std::size_t size() const noexcept { return this->sz; }

    type * data() noexcept { return this->p; }
    constexpr type const * data() const noexcept { return this->p; }

    type * begin() { return this->p; }
    type * end() { return this->p + this->sz; }
    constexpr type const * begin() const { return this->p; }
    constexpr type const * end() const { return this->p + this->sz; }

private:
    type * p;
    std::size_t sz;
};

template<class T>
struct array_view<T const>
{
    using type = T const;

    constexpr array_view() = default;

    constexpr array_view(std::nullptr_t)
    : array_view(nullptr, 0)
    {}

    constexpr array_view(type * p, std::size_t sz)
    : p(p)
    , sz(sz)
    {}

    template<class Cont>
    constexpr array_view(Cont & cont)
    : array_view(cont.data(), cont.size())
    {}

    template<std::size_t N>
    constexpr array_view(T (&a)[N])
    : array_view(a, N)
    {}

    template<std::size_t N>
    constexpr array_view(type (&a)[N])
    : array_view(a, N)
    {}

    constexpr array_view(array_view const &) = default;
    constexpr array_view(array_view<T> const & other)
    : array_view(other.data(), other.size())
    {}

    array_view & operator=(array_view const &) = default;
    array_view & operator=(array_view<T> const & other) {
        this->p = other.data();
        this->sz = other.size();
        return *this;
    }

    constexpr explicit operator bool () const noexcept { return this->p; }

    constexpr std::size_t size() const noexcept { return this->sz; }

    constexpr type * data() const noexcept { return this->p; }

    constexpr type * begin() const { return this->p; }
    constexpr type * end() const { return this->p + this->sz; }

private:
    type * p;
    std::size_t sz;
};


template<class T>
array_view<T> make_array_view(T * x, std::size_t n)
{ return {x, n}; }

template<class T, std::size_t N>
array_view<T> make_array_view(T (&arr)[N])
{ return {arr, N}; }

template<class Cont>
auto make_array_view(Cont & cont) -> array_view<decltype(cont.data())>
{ return {cont.data(), cont.size()}; }


template<class T>
array_view<T const> make_const_array_view(T const * x, std::size_t n)
{ return {x, n}; }

template<class T, std::size_t N>
array_view<T const> make_const_array_view(T const (&arr)[N])
{ return {arr, N}; }


template<std::size_t N>
array_view<char const> cstr_array_view(char const (&str)[N])
{ return {str, N-1}; }

template<std::size_t N>
array_view<char> cstr_array_view(char (&str)[N])
{ return {str, N-1}; }


using array_u8 = array_view<uint8_t>;
using array_u16 = array_view<uint16_t>;
using array_u32 = array_view<uint32_t>;
using array_u64 = array_view<uint64_t>;
using array_const_u8 = array_view<uint8_t const>;
using array_const_u16 = array_view<uint16_t const>;
using array_const_u32 = array_view<uint32_t const>;
using array_const_u64 = array_view<uint64_t const>;

using array_s8 = array_view<int8_t>;
using array_s16 = array_view<int16_t>;
using array_s32 = array_view<int32_t>;
using array_s64 = array_view<int64_t>;
using array_const_s8 = array_view<int8_t const>;
using array_const_s16 = array_view<int16_t const>;
using array_const_s32 = array_view<int32_t const>;
using array_const_s64 = array_view<int64_t const>;

using array_char = array_view<char>;
using array_const_char = array_view<char const>;

#endif
