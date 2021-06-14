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

    template<class Seq, class T, class = void, class = std::true_type>
    struct sized_sequence_of_impl
    {};

    template<class Seq, class = void>
    struct is_bounded_sequence_impl
    : std::false_type
    {};
} // namespace detail


// has value member when T is a size_bounds_sequence
template<class T>
using sequence_to_size_bounds_t = typename detail::sequence_to_size_bounds_impl<
    std::remove_cv_t<std::remove_reference_t<T>>
>::type;

// has value member when Seq is a sized_sequence of type T
template<class Seq, class T>
using sized_sequence_of = detail::sized_sequence_of_impl<
    std::remove_cv_t<std::remove_reference_t<Seq>>, T
>;

template<class Seq>
using is_bounded_sequence = typename detail::is_bounded_sequence_impl<
    std::remove_cv_t<std::remove_reference_t<Seq>>
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


    template<class Seq>
    struct is_bounded_sequence_impl<Seq,
        std::void_t<typename sequence_to_size_bounds_impl<Seq>::type>
    >
    : std::true_type
    {};


    template<class T>
    struct size_bounds_to_static_size
    {};

    template<std::size_t N>
    struct size_bounds_to_static_size<size_bounds<N, N>>
    : std::integral_constant<std::size_t, N>
    {};

    template<class T>
    using size_bounds_to_static_size_t = typename size_bounds_to_static_size<T>::type;

    template<class Seq, class T>
    struct sized_sequence_of_impl<Seq, T,
        std::void_t<size_bounds_to_static_size_t<
            typename detail::sequence_to_size_bounds_impl<Seq>::type>>,
        typename std::is_same<std::remove_cv_t<std::remove_reference_t<
            decltype(*utils::begin(std::declval<Seq&>()))>>, T>::type
    >
    : size_bounds_to_static_size_t<
        typename detail::sequence_to_size_bounds_impl<Seq>::type>
    {};

    template<class Seq, class T>
    struct sized_sequence_of_impl<Seq, T&,
        std::void_t<size_bounds_to_static_size_t<
            typename detail::sequence_to_size_bounds_impl<Seq>::type>>,
        typename std::is_same<decltype(*utils::begin(std::declval<Seq&>())), T&>::type
    >
    : size_bounds_to_static_size_t<
        typename detail::sequence_to_size_bounds_impl<Seq>::type>
    {};

    template<class Seq, class T>
    struct sized_sequence_of_impl<Seq, T&&,
        std::void_t<size_bounds_to_static_size_t<
            typename detail::sequence_to_size_bounds_impl<Seq>::type>>,
        typename std::is_same<decltype(*utils::begin(std::declval<Seq&>())), T&&>::type
    >
    : size_bounds_to_static_size_t<
        typename detail::sequence_to_size_bounds_impl<Seq>::type>
    {};
} // namespace detail
