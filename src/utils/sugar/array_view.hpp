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

#pragma once

#include <type_traits>

#include <cstdint>
#include <cassert>

#include "utils/sugar/array.hpp"

#include "cxx/cxx.hpp"


template<class T>
struct array_view
{
    using type = T;
    using iterator = T *;
    using const_iterator = T *;
    using value_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
    using reference = T&;
    using const_reference = T const &;

    constexpr array_view() = default;
    constexpr array_view(array_view const & other) = default;
    array_view & operator = (array_view const & other) = default;

    constexpr array_view(std::nullptr_t /*null*/) noexcept
    : array_view(nullptr, nullptr)
    {}

    constexpr array_view(type * p, std::size_t sz) noexcept
    : p(p)
    , sz(sz)
    {}

    constexpr array_view(type * p, type * pright) noexcept
    : p(p)
    , sz(pright - p)
    {}

    template<class U, class = decltype(
        *static_cast<type**>(nullptr) = utils::data(std::declval<U&>()),
        *static_cast<std::size_t*>(nullptr) = utils::size(std::declval<U&>())
    )>
    constexpr array_view(U & x)
    noexcept(noexcept((void(utils::data(x)), utils::size(x))))
    : p(utils::data(x))
    , sz(utils::size(x))
    {}

    template<class U, class = decltype(
        *static_cast<type**>(nullptr) = static_cast<U*>(nullptr)
    )>
    constexpr array_view(array_view<U> av) noexcept
    : p(av.data())
    , sz(av.size())
    {}

    constexpr bool empty() const noexcept { return !this->sz; }

    constexpr std::size_t size() const noexcept { return this->sz; }

    constexpr type & front() const noexcept { /*assert(this->size());*/ return *this->p; }
    constexpr type & back() const noexcept { /*assert(this->size());*/ return this->p[this->sz-1u]; }
    REDEMPTION_CONSTEXPR_AFTER_CXX11 type & front() noexcept { assert(this->size()); return *this->p; }
    REDEMPTION_CONSTEXPR_AFTER_CXX11 type & back() noexcept { assert(this->size()); return this->p[this->sz-1u]; }

    constexpr type const * data() const noexcept { return this->p; }
    REDEMPTION_CONSTEXPR_AFTER_CXX11 type * data() noexcept { return this->p; }

    constexpr type const * begin() const noexcept { return this->data(); }
    constexpr type const * end() const noexcept { return this->data() + this->size(); }
    REDEMPTION_CONSTEXPR_AFTER_CXX11 type * begin() noexcept { return this->data(); }
    REDEMPTION_CONSTEXPR_AFTER_CXX11 type * end() noexcept { return this->data() + this->size(); }

    REDEMPTION_CONSTEXPR_AFTER_CXX11 type & operator[](std::size_t i) noexcept
    { assert(i < this->size()); return this->data()[i]; }
    REDEMPTION_CONSTEXPR_AFTER_CXX11 type const & operator[](std::size_t i) const noexcept
    { assert(i < this->size()); return this->data()[i]; }

    REDEMPTION_CONSTEXPR_AFTER_CXX11 array_view first(std::size_t n) const noexcept
    {
        assert(n <= this->size());
        return {this->data(), n};
    }

    REDEMPTION_CONSTEXPR_AFTER_CXX11 array_view last(std::size_t n) const noexcept
    {
        assert(n <= this->size());
        return {this->data() + this->size() - n, n};
    }


    REDEMPTION_CONSTEXPR_AFTER_CXX11 array_view subarray(std::size_t offset) noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, static_cast<std::size_t>(this->size() - offset)};
    }

    REDEMPTION_CONSTEXPR_AFTER_CXX11 array_view subarray(std::size_t offset, std::size_t count) noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    REDEMPTION_CONSTEXPR_AFTER_CXX11 array_view<T const> subarray(std::size_t offset) const noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, static_cast<std::size_t>(this->size() - offset)};
    }

    REDEMPTION_CONSTEXPR_AFTER_CXX11 array_view<T const> subarray(std::size_t offset, std::size_t count) const noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

private:
    type * p        = nullptr;
    std::size_t sz  = 0;
};


template<class T>
constexpr array_view<T> make_array_view(array_view<T> av) noexcept
{ return av; }

template<class T>
constexpr array_view<T> make_array_view(T * x, std::size_t n) noexcept
{ return {x, n}; }

template<class T>
constexpr array_view<T> make_array_view(T * left, T * right) noexcept
{ return {left, right}; }

template<class T>
constexpr array_view<const T> make_array_view(T const * left, T * right) noexcept
{ return {left, right}; }

template<class T>
constexpr array_view<const T> make_array_view(T * left, T const * right) noexcept
{ return {left, right}; }

template<class T, std::size_t N>
constexpr array_view<T> make_array_view(T (&arr)[N]) noexcept
{ return {arr, N}; }

template<class Cont>
constexpr auto make_array_view(Cont & cont)
noexcept(noexcept(array_view<typename std::remove_pointer<decltype(cont.data())>::type>{cont}))
-> array_view<typename std::remove_pointer<decltype(cont.data())>::type>
{ return {cont}; }

template<class T>
constexpr array_view<T const> make_const_array_view(array_view<T> av) noexcept
{ return av; }

template<class T>
constexpr array_view<T const> make_const_array_view(T const * x, std::size_t n) noexcept
{ return {x, n}; }

template<class T>
constexpr array_view<const T> make_const_array_view(T const * left, T const * right) noexcept
{ return {left, right}; }

template<class T, std::size_t N>
constexpr array_view<T const> make_const_array_view(T const (&arr)[N]) noexcept
{ return {arr, N}; }


// TODO renamed to zstring_array
template<std::size_t N>
constexpr array_view<char const> cstr_array_view(char const (&str)[N]) noexcept
{ return {str, N-1}; }

// TODO renamed to zstring_array
// forbidden: array_view is for litterals
template<std::size_t N>
array_view<char> cstr_array_view(char (&str)[N]) = delete;


using array_view_u8 = array_view<uint8_t>;
using array_view_u16 = array_view<uint16_t>;
using array_view_u32 = array_view<uint32_t>;
using array_view_u64 = array_view<uint64_t>;
using array_view_const_u8 = array_view<uint8_t const>;
using array_view_const_u16 = array_view<uint16_t const>;
using array_view_const_u32 = array_view<uint32_t const>;
using array_view_const_u64 = array_view<uint64_t const>;

using array_view_s8 = array_view<int8_t>;
using array_view_s16 = array_view<int16_t>;
using array_view_s32 = array_view<int32_t>;
using array_view_s64 = array_view<int64_t>;
using array_view_const_s8 = array_view<int8_t const>;
using array_view_const_s16 = array_view<int16_t const>;
using array_view_const_s32 = array_view<int32_t const>;
using array_view_const_s64 = array_view<int64_t const>;

using array_view_char = array_view<char>;
using array_view_const_char = array_view<char const>;

