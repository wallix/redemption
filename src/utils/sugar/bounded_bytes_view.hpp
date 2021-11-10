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

#include "utils/sugar/byte_ptr.hpp"
#include "utils/sugar/bounded_array_view.hpp"

/**
 * \c bounded_array_view on \c uint8_t* and \c char*
 */
template<std::size_t AtLeast, std::size_t AtMost>
struct writable_bounded_bytes_view : writable_bounded_array_view<uint8_t, AtLeast, AtMost>
{
    using writable_bounded_u8_array_view = writable_bounded_array_view<uint8_t, AtLeast, AtMost>;
    using writable_bounded_chars_view = writable_bounded_array_view<char, AtLeast, AtMost>;

    writable_bounded_bytes_view() = delete;
    writable_bounded_bytes_view(writable_bounded_bytes_view &&) = default;
    writable_bounded_bytes_view(writable_bounded_bytes_view const &) = default;
    writable_bounded_bytes_view & operator=(writable_bounded_bytes_view &&) = default;
    writable_bounded_bytes_view & operator=(writable_bounded_bytes_view const &) = default;


    template<class T, std::size_t n>
    writable_bounded_bytes_view(T(&)[n]) = delete;

    template<class T, std::size_t n>
    writable_bounded_bytes_view & operator=(T(&)[n]) = delete;

    writable_bounded_bytes_view(writable_bounded_chars_view av) noexcept /*NOLINT*/
    : writable_bounded_u8_array_view(
        writable_bounded_u8_array_view::assumed(byte_ptr_cast(av.data())))
    {}

    writable_bounded_bytes_view(writable_bounded_u8_array_view av) noexcept /*NOLINT*/
    : writable_bounded_u8_array_view(av)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<writable_bounded_u8_array_view, U&&>::value, bool
    >::type = 1>
    explicit constexpr writable_bounded_bytes_view(U && a) noexcept /*NOLINT*/
    : writable_bounded_u8_array_view(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<writable_bounded_chars_view, U&&>::value, bool
    >::type = 1>
    explicit constexpr writable_bounded_bytes_view(U && a) noexcept /*NOLINT*/
    : writable_bounded_bytes_view(writable_bounded_chars_view(a))
    {}


    [[nodiscard]] char       * as_charp() noexcept { return char_ptr_cast(this->data()); }
    [[nodiscard]] char const * as_charp() const noexcept { return char_ptr_cast(this->data()); }
    [[nodiscard]] constexpr uint8_t       * as_u8p() noexcept { return this->data(); }
    [[nodiscard]] constexpr uint8_t const * as_u8p() const noexcept { return this->data(); }

    [[nodiscard]] writable_bounded_chars_view as_chars() noexcept
    {
        return writable_bounded_chars_view::assumed(this->as_charp());
    }

    [[nodiscard]] bounded_array_view<char, AtLeast, AtMost> as_chars() const noexcept
    {
        return bounded_array_view<char, AtLeast, AtMost>::assumed(this->as_charp());
    }
};

template<class T, class Bounds = sequence_to_size_bounds_t<T>>
writable_bounded_bytes_view(T&&) -> writable_bounded_bytes_view<
    Bounds::at_least, Bounds::at_most
>;


/**
 * \c bounded_array_view on constant \c uint8_t* and \c char*
 */
template<std::size_t AtLeast, std::size_t AtMost>
struct bounded_bytes_view : bounded_array_view<uint8_t, AtLeast, AtMost>
{
    using bounded_u8_array_view = bounded_array_view<uint8_t, AtLeast, AtMost>;
    using bounded_chars_view = bounded_array_view<char, AtLeast, AtMost>;

    bounded_bytes_view() = delete;
    bounded_bytes_view(bounded_bytes_view &&) = default;
    bounded_bytes_view(bounded_bytes_view const &) = default;
    bounded_bytes_view & operator=(bounded_bytes_view &&) = default;
    bounded_bytes_view & operator=(bounded_bytes_view const &) = default;


    template<class T, std::size_t n>
    bounded_bytes_view(T(&)[n]) = delete;

    template<class T, std::size_t n>
    bounded_bytes_view & operator=(T(&)[n]) = delete;

    bounded_bytes_view(bounded_chars_view av) noexcept /*NOLINT*/
    : bounded_u8_array_view(byte_ptr_cast(av.data()), av.size())
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<bounded_u8_array_view, U&&>::value, bool
    >::type = 1>
    constexpr bounded_bytes_view(U && a) noexcept(noexcept(bounded_u8_array_view(a))) /*NOLINT*/
    : bounded_u8_array_view(a)
    {}

    template<class U, typename std::enable_if<
      std::is_constructible<bounded_chars_view, U&&>::value, bool
    >::type = 1>
    constexpr bounded_bytes_view(U && a) noexcept(noexcept(bounded_chars_view(a))) /*NOLINT*/
    : bounded_bytes_view(bounded_chars_view(a))
    {}


    [[nodiscard]] char const * as_charp() const noexcept { return char_ptr_cast(this->data()); }
    [[nodiscard]] constexpr uint8_t const * as_u8p() const noexcept { return this->data(); }

    [[nodiscard]] bounded_chars_view as_chars() const noexcept
    {
        return bounded_chars_view::assumed(this->as_charp());
    }
};

template<class T, class Bounds = sequence_to_size_bounds_t<T>>
bounded_bytes_view(T&&) -> bounded_bytes_view<
    Bounds::at_least, Bounds::at_most
>;


template<std::size_t N>
using sized_bytes_view = bounded_bytes_view<N, N>;

template<std::size_t N>
using writable_sized_bytes_view = writable_bounded_bytes_view<N, N>;


namespace detail
{
    template<std::size_t AtLeast, std::size_t AtMost>
    struct sequence_to_size_bounds_impl<writable_bounded_bytes_view<AtLeast, AtMost>>
    {
        using type = size_bounds<AtLeast, AtMost>;
    };

    template<std::size_t AtLeast, std::size_t AtMost>
    struct sequence_to_size_bounds_impl<bounded_bytes_view<AtLeast, AtMost>>
    {
        using type = size_bounds<AtLeast, AtMost>;
    };
}
