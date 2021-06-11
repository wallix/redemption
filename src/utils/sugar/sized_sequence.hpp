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
#include "utils/sugar/bounded_sequence.hpp"


namespace detail
{
    template<class Seq, class T, class = void, class = std::true_type>
    struct is_sized_sequence_of_impl
    : std::false_type
    {};


    template<class T>
    struct size_bounds_to_static_size
    {};

    template<class T>
    using size_bounds_to_static_size_t = typename size_bounds_to_static_size<T>::type;


    template<std::size_t N>
    struct size_bounds_to_static_size<size_bounds<N, N>>
    : std::integral_constant<std::size_t, N>
    {};
} // namespace detail


template<class Seq, class T>
using is_sized_sequence_of = typename detail::is_sized_sequence_of_impl<
    std::remove_cv_t<std::remove_reference_t<Seq>>, T
>::type;

// has size_type and size members when Seq is a sized_sequence of type T
template<class Seq, class T, class = std::true_type>
struct sized_sequence_of
{};

template<class Seq, class T>
struct sized_sequence_of<Seq, T, is_sized_sequence_of<Seq, T>>
{
    using size_type = detail::size_bounds_to_static_size_t<sequence_to_size_bounds_t<Seq>>;
    static constexpr std::size_t size = size_type::value;
};


namespace detail
{
    template<class Seq, class T>
    struct is_sized_sequence_of_impl<Seq, T,
        std::void_t<size_bounds_to_static_size_t<sequence_to_size_bounds_t<Seq>>>,
        typename std::is_same<std::remove_cv_t<std::remove_reference_t<
            decltype(*utils::begin(std::declval<Seq&>()))>>, T>::type
    >
    : std::true_type
    {};

    template<class Seq, class T>
    struct is_sized_sequence_of_impl<Seq, T&,
        std::void_t<size_bounds_to_static_size_t<sequence_to_size_bounds_t<Seq>>>,
        typename std::is_same<decltype(*utils::begin(std::declval<Seq&>())), T&>::type
    >
    : std::true_type
    {};

    template<class Seq, class T>
    struct is_sized_sequence_of_impl<Seq, T&&,
        std::void_t<size_bounds_to_static_size_t<sequence_to_size_bounds_t<Seq>>>,
        typename std::is_same<decltype(*utils::begin(std::declval<Seq&>())), T&&>::type
    >
    : std::true_type
    {};
} // namespace detail
