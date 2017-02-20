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

#include <type_traits>
#include <array>

namespace utils
{
    template<std::size_t N, class T>
    constexpr std::integral_constant<std::size_t, N>
    size(T const (&)[N])
    { return {}; }

    template<std::size_t N, class T>
    constexpr std::integral_constant<std::size_t, N>
    size(std::array<T, N> const &)
    { return {}; }

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
