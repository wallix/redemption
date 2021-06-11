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

#include "utils/sugar/bounded_array_view.hpp"
#include "utils/sugar/sized_sequence.hpp"


template<class T, std::size_t Size>
struct sized_array_view : bounded_array_view<T, Size, Size>
{
    using element_type = T const;
    using value_type = std::remove_cv_t<T>;
    using reference = T const&;
    using iterator = T const*;
    using pointer = T const*;
    using const_reference = T const&;
    using const_iterator = T const*;
    using const_pointer = T const*;
    using size_type = std::size_t;

    static constexpr std::size_t extent = Size;

    template<class C, class = decltype(void(
        bounded_array_view<T, Size, Size>(std::declval<C&&>())
    ))>
    constexpr sized_array_view(C&& a) /* NOLINT(bugprone-forwarding-reference-overload) */
        noexcept(noexcept(array_view<T>(static_cast<C&&>(a))))
    : bounded_array_view<T, Size, Size>(static_cast<C&&>(a))
    {}
};


template<class T, class Bounds = sequence_to_size_bounds_t<T>>
sized_array_view(T&&) -> sized_array_view<
    detail::value_type_array_view_from_t<T&&>,
    detail::size_bounds_to_static_size<Bounds>::value
>;


template<class T, std::size_t Size>
struct writable_sized_array_view : writable_bounded_array_view<T, Size, Size>
{
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using reference = T&;
    using iterator = T*;
    using pointer = T*;
    using const_reference = T const&;
    using const_iterator = T const*;
    using const_pointer = T const*;
    using size_type = std::size_t;

    static constexpr std::size_t extent = Size;

    template<class C, class = decltype(void(
        writable_bounded_array_view<T, Size, Size>(std::declval<C&&>())
    ))>
    explicit constexpr writable_sized_array_view(C&& a) /* NOLINT(bugprone-forwarding-reference-overload) */
        noexcept(noexcept(writable_array_view<T>(static_cast<C&&>(a))))
    : writable_bounded_array_view<T, Size, Size>(static_cast<C&&>(a))
    {}

    template<class U, std::size_t AtLeast, std::size_t AtMost, class = std::enable_if_t<
        Size <= AtLeast && Size <= AtMost, decltype(void(
            *static_cast<pointer*>(nullptr) = static_cast<U*>(nullptr)
        ))
    >>
    constexpr writable_sized_array_view(
        writable_bounded_array_view<U, AtLeast, AtMost> a
    ) noexcept
    : writable_bounded_array_view<T, Size, Size>(a)
    {}
};


template<class T, class Bounds = sequence_to_size_bounds_t<T>>
writable_sized_array_view(T&&) -> writable_sized_array_view<
    detail::value_type_array_view_from_t<T&&>,
    detail::size_bounds_to_static_size<Bounds>::value
>;


namespace detail
{
    template<class T, std::size_t Size>
    struct sequence_to_size_bounds_impl<sized_array_view<T, Size>>
    {
        using type = size_bounds<Size, Size>;
    };

    template<class T, std::size_t Size>
    struct sequence_to_size_bounds_impl<writable_sized_array_view<T, Size>>
    {
        using type = size_bounds<Size, Size>;
    };
} // namespace detail


template<class T, std::size_t Size>
constexpr sized_array_view<T, Size>
make_sized_array_view(sized_array_view<T, Size> av) noexcept
{
    return av;
}

template<class T, std::size_t Size>
constexpr sized_array_view<T, Size>
make_sized_array_view(writable_sized_array_view<T, Size> av) noexcept
{
    return av;
}

template<class Cont>
constexpr auto make_sized_array_view(Cont const& cont)
    noexcept(noexcept(sized_array_view{cont}))
-> decltype(sized_array_view{cont})
{
    return {cont};
}

template<class T, std::size_t N>
constexpr sized_array_view<T, N>
make_sized_array_view(T const (&arr)[N]) noexcept
{
    return sized_array_view<T, N>::assumed(&arr[0]);
}


// TODO renamed to zstring_array
template<std::size_t N>
constexpr sized_array_view<char, N-1>
cstr_sized_array_view(char const (&str)[N]) noexcept
{
    return sized_array_view<char, N-1>::assumed(str);
}

// TODO renamed to zstring_array
// forbidden: sized_array_view is for litterals
template<std::size_t N>
sized_array_view<char, N-1> cstr_sized_array_view(char (&str)[N]) = delete;


template<class T, std::size_t Size>
constexpr writable_sized_array_view<T, Size>
make_writable_sized_array_view(writable_sized_array_view<T, Size> av) noexcept
{
    return av;
}

template<class Cont>
constexpr auto make_writable_sized_array_view(Cont& cont)
    noexcept(noexcept(writable_sized_array_view{cont}))
-> decltype(writable_sized_array_view{cont})
{
    return writable_sized_array_view{cont};
}

template<class T, std::size_t N>
constexpr writable_sized_array_view<T, N>
make_writable_sized_array_view(T (&arr)[N]) noexcept
{
    return writable_sized_array_view<T, N>::assumed(&arr[0]);
}


template<std::size_t N, class T, class AV
    = sized_array_view<detail::value_type_array_view_from_t<T&&>, N>>
constexpr AV make_assumed_sized_array_view(T&& x)
    noexcept(noexcept(AV::assumed(static_cast<T&&>(x))))
{
    return AV::assumed(static_cast<T&&>(x));
}

template<std::size_t N, class T>
constexpr sized_array_view<T, N> make_assumed_sized_array_view(T const* x) noexcept
{
    return sized_array_view<T, N>::assumed(x);
}

template<std::size_t N, class T, class AV
    = writable_sized_array_view<detail::value_type_array_view_from_t<T&&>, N>>
constexpr AV make_assumed_writable_sized_array_view(T&& x)
    noexcept(noexcept(AV::assumed(static_cast<T&&>(x))))
{
    return AV::assumed(static_cast<T&&>(x));
}

template<std::size_t N, class T>
constexpr writable_sized_array_view<T, N>
make_assumed_writable_sized_array_view(T* x) noexcept
{
    return writable_sized_array_view<T, N>::assumed(x);
}


template<std::size_t N> using sized_chars_view = sized_array_view<char, N>;
template<std::size_t N> using sized_u8_array_view = sized_array_view<std::uint8_t, N>;
template<std::size_t N> using sized_writable_u8_array_view = writable_sized_array_view<std::uint8_t, N>;
