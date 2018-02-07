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

#include "cxx/cxx.hpp"

#include <type_traits>
#include <array>
#if __cplusplus >= REDEMPTION_CXX_STD_17
# include <iterator>
#endif

namespace utils
{
    namespace adl_barrier_ {
#if __cplusplus < REDEMPTION_CXX_STD_17
        template<class T, std::size_t n>
        constexpr T * data(T (&a)[n]) noexcept
        { return a; }

        template<class C>
        constexpr auto data(C & c)
        noexcept(noexcept(c.data()))
        -> decltype(c.data())
        { return c.data(); }

        template<class T, std::size_t n>
        constexpr std::size_t size(T (&)[n]) noexcept
        { return n; }

        template<class C>
        constexpr auto size(C & c)
        noexcept(noexcept(c.size()))
        -> decltype(c.size())
        { return c.size(); }
# else
        using std::data;
        using std::size;
# endif
    }

    namespace detail_
    {
        using adl_barrier_::data;
        using adl_barrier_::size;

        template<class C>
        constexpr auto data_impl(C & c)
        noexcept(noexcept(data(c)))
        -> decltype(data(c))
        { return data(c); }

        template<class C>
        constexpr auto size_impl(C & c)
        noexcept(noexcept(size(c)))
        -> decltype(size(c))
        { return size(c); }
    }

    template<class C>
    constexpr auto data(C & c)
    noexcept(noexcept(detail_::data_impl(c)))
    -> decltype(detail_::data_impl(c))
    { return detail_::data_impl(c); }

    template<class C>
    constexpr auto size(C & c)
    noexcept(noexcept(detail_::size_impl(c)))
    -> decltype(static_cast<std::size_t>(detail_::size_impl(c)))
    { return static_cast<std::size_t>(detail_::size_impl(c)); }


    template<std::size_t N, class T>
    constexpr T const &
    back(T const (&a)[N])
    { return a[N-1]; }

    template<std::size_t N, class T>
    constexpr T &
    back(T (& a)[N])
    { return a[N-1]; }

    template<class T, class... Ts>
    constexpr
    std::array<T, sizeof...(Ts)>
    make_array(Ts && ... values)
    {
        using array_type = std::array<T, sizeof...(Ts)>;
        return array_type{{T(std::forward<Ts>(values))...}};
    }

    template<class T, class... Ts>
    constexpr
    std::array<typename std::decay<T>::type, sizeof...(Ts) + 1>
    make_array(T && value, Ts && ... values)
    {
        using array_type = std::array<typename std::decay<T>::type, sizeof...(Ts) + 1>;
        return array_type{{std::forward<T>(value), std::forward<Ts>(values)...}};
    }
}
