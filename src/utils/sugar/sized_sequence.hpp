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
    template<class T>
    struct sequence_to_static_size_impl
    {};

    template<class T, std::size_t N>
    struct sequence_to_static_size_impl<T[N]>
    : std::integral_constant<std::size_t, N>
    {};

    template<class T, std::size_t N>
    struct sequence_to_static_size_impl<std::array<T, N>>
    : std::integral_constant<std::size_t, N>
    {};

    template<class Seq, class T, class = void, class = std::true_type>
    struct is_static_sized_sequence_of_impl
    : std::false_type
    {};
} // namespace detail


// has value member when T is a static_sized_sequence
template<class T>
using sequence_to_static_size_t = typename detail::sequence_to_static_size_impl<
    std::remove_cv_t<std::remove_reference_t<T>>
>::type;

template<class T>
using value_type_of_sequence_t = std::remove_pointer_t<
    decltype(utils::data(std::declval<T&>()))
>;

template<class Seq, class T>
using is_static_sized_sequence_of = typename detail::is_static_sized_sequence_of_impl<
    std::remove_cv_t<std::remove_reference_t<Seq>>, T
>::type;

// has size_type and size members when Seq is a static_sized_sequence of type T
template<class Seq, class T, class = std::true_type>
struct static_sized_sequence_of
{};

template<class Seq, class T>
struct static_sized_sequence_of<Seq, T, is_static_sized_sequence_of<Seq, T>>
{
    using size_type = sequence_to_static_size_t<Seq>;
    static constexpr std::size_t size = size_type::value;
};


namespace detail
{
    template<class Seq, class T>
    struct is_static_sized_sequence_of_impl<Seq, T,
        std::void_t<typename sequence_to_static_size_impl<Seq>::type>,
        typename std::is_same<std::remove_cv_t<std::remove_reference_t<
            decltype(*utils::begin(std::declval<Seq&>()))>>, T>::type
    >
    : std::true_type
    {};

    template<class Seq, class T>
    struct is_static_sized_sequence_of_impl<Seq, T&,
        std::void_t<typename sequence_to_static_size_impl<Seq>::type>,
        typename std::is_same<decltype(*utils::begin(std::declval<Seq&>())), T&>::type
    >
    : std::true_type
    {};

    template<class Seq, class T>
    struct is_static_sized_sequence_of_impl<Seq, T&&,
        std::void_t<typename sequence_to_static_size_impl<Seq>::type>,
        typename std::is_same<decltype(*utils::begin(std::declval<Seq&>())), T&&>::type
    >
    : std::true_type
    {};
} // namespace detail
