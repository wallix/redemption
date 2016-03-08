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

    constexpr array_view(type * p, std::size_t sz)
    : p(p)
    , sz(sz)
    {}

    constexpr array_view(type * p, type * pright)
    : p(p)
    , sz(pright - p)
    {}

    template<std::size_t N>
    constexpr array_view(type (&a)[N])
    : p(a)
    , sz(N)
    {}

    template<class U, class = decltype(
        *static_cast<type**>(nullptr) = static_cast<U*>(nullptr)->data(),
        *static_cast<std::size_t*>(nullptr) = static_cast<U*>(nullptr)->size()
    )>
    constexpr array_view(U & x)
    : p(x.data())
    , sz(x.size())
    {}

    constexpr bool empty() const noexcept { return this->sz; }

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

    constexpr array_view(type * p, std::size_t sz)
    : p(p)
    , sz(sz)
    {}

    constexpr array_view(type * p, type * pright)
    : p(p)
    , sz(pright - p)
    {}

    template<std::size_t N>
    constexpr array_view(type (&a)[N])
    : p(a)
    , sz(N)
    {}

    template<class U, class = decltype(
        *static_cast<type**>(nullptr) = static_cast<U*>(nullptr)->data(),
        *static_cast<std::size_t*>(nullptr) = static_cast<U*>(nullptr)->size()
    )>
    constexpr array_view(U & x)
    : p(x.data())
    , sz(x.size())
    {}

    constexpr bool empty() const noexcept { return this->sz; }

    constexpr std::size_t size() const noexcept { return this->sz; }

    constexpr type * data() const noexcept { return this->p; }

    constexpr type * begin() const { return this->p; }
    constexpr type * end() const { return this->p + this->sz; }

private:
    type * p;
    std::size_t sz;
};


template<class T>
constexpr array_view<T> make_array_view(T * x, std::size_t n)
{ return {x, n}; }

template<class T>
constexpr array_view<T> make_array_view(T * left, T * right)
{ return {left, right}; }

template<class T>
constexpr array_view<const T> make_array_view(T const * left, T * right)
{ return {left, right}; }

template<class T>
constexpr array_view<const T> make_array_view(T * left, T const * right)
{ return {left, right}; }

template<class T, std::size_t N>
constexpr array_view<T> make_array_view(T (&arr)[N])
{ return {arr, N}; }

template<class Cont>
constexpr auto make_array_view(Cont & cont) -> array_view<decltype(cont.data())>
{ return {cont.data(), cont.size()}; }

template<class T>
constexpr array_view<T const> make_const_array_view(T const * x, std::size_t n)
{ return {x, n}; }

template<class T>
constexpr array_view<const T> make_const_array_view(T const * left, T const * right)
{ return {left, right}; }

template<class T, std::size_t N>
constexpr array_view<T const> make_const_array_view(T const (&arr)[N])
{ return {arr, N}; }


template<std::size_t N>
constexpr array_view<char const> cstr_array_view(char const (&str)[N])
{ return {str, N-1}; }

template<std::size_t N>
array_view<char> cstr_array_view(char (&str)[N])
{ return {str, N-1}; }

#endif
