/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include <type_traits>
#include <array>

#include "utils/sugar/array.hpp"


namespace detail
{
    template<std::size_t AtLeast, std::size_t AtMost>
    struct size_bounds
    {
        static constexpr std::size_t at_least = AtLeast;
        static constexpr std::size_t at_most = AtMost;
    };

    template<class T>
    struct sequence_to_size_bounds_impl
    {};
} // namespace detail


// has value member when T is a size_bounds_sequence
template<class T>
using sequence_to_size_bounds_t = typename detail::sequence_to_size_bounds_impl<
    std::remove_cv_t<std::remove_reference_t<T>>
>::type;


namespace detail
{
    template<class T, std::size_t N>
    struct sequence_to_size_bounds_impl<T[N]>
    {
        using type = size_bounds<N, N>;
    };

    template<class T, std::size_t N>
    struct sequence_to_size_bounds_impl<std::array<T, N>>
    {
        using type = size_bounds<N, N>;
    };
} // namespace detail
