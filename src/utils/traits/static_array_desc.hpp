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

#include "utils/sugar/bounded_sequence.hpp"

template<bool IsStatic, std::size_t Size>
struct static_array_desc
{
    static const bool is_static = IsStatic;
    static const std::size_t size = Size;
};

template<class Bounds>
using bounds_to_static_array_desc
    = static_array_desc<Bounds::at_least == Bounds::at_most, Bounds::at_most>;

namespace detail
{
    template<class T, class = void>
    struct to_static_array_desc_impl
    {
        using type = static_array_desc<false, 0>;
    };

    template<class T>
    struct to_static_array_desc_impl<T, std::void_t<
        typename sequence_to_size_bounds_impl<T>::type
    >>
    {
        using type = bounds_to_static_array_desc<typename sequence_to_size_bounds_impl<T>::type>;
    };
} // namespace detail

template<class T>
using to_static_array_desc = typename detail::to_static_array_desc_impl<
    std::remove_cv_t<std::remove_reference_t<T>>
>::type;
