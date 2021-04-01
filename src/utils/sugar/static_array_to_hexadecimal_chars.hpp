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

#include <array>
#include <cstdint>

#include "utils/sugar/int_to_chars.hpp"

namespace detail
{
    struct static_array_to_hexadecimal_chars_result_access;
}

template<std::size_t N>
struct static_array_to_hexadecimal_chars_result
{
    static_array_to_hexadecimal_chars_result() = default;

    char const* data() const noexcept { return buffer; }
    std::size_t size() const noexcept { return N*2; }

private:
    friend detail::static_array_to_hexadecimal_chars_result_access;

    char buffer[N*2];
};

template<std::size_t N>
struct static_array_to_hexadecimal_zchars_result
{
    static_array_to_hexadecimal_zchars_result() = default;

    char const* data() const noexcept { return buffer; }
    std::size_t size() const noexcept { return N*2; }

    char const* c_str() const noexcept { return buffer; }

private:
    friend detail::static_array_to_hexadecimal_chars_result_access;

    char buffer[N*2+1];
};


template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
static_array_to_hexadecimal_upper_chars(uint8_t const (&a)[N]) noexcept;

template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
static_array_to_hexadecimal_lower_chars(uint8_t const (&a)[N]) noexcept;

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
static_array_to_hexadecimal_upper_zchars(uint8_t const (&a)[N]) noexcept;

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
static_array_to_hexadecimal_lower_zchars(uint8_t const (&a)[N]) noexcept;


template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
static_array_to_hexadecimal_upper_chars(std::array<uint8_t, N> const& a) noexcept;

template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
static_array_to_hexadecimal_lower_chars(std::array<uint8_t, N> const& a) noexcept;

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
static_array_to_hexadecimal_upper_zchars(std::array<uint8_t, N> const& a) noexcept;

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
static_array_to_hexadecimal_lower_zchars(std::array<uint8_t, N> const& a) noexcept;


namespace detail
{
    struct static_array_to_hexadecimal_chars_result_access
    {
        template<class T>
        inline static char* buffer(T& r) noexcept
        {
            return r.buffer;
        }
    };

    template<std::size_t N>
    struct static_array_u8_view
    {
        uint8_t const* p;

        uint8_t const* begin() const noexcept { return p; }
        uint8_t const* end() const noexcept { return p + N; }
    };

    template<std::size_t N>
    static_array_to_hexadecimal_chars_result<N>
    static_array_to_hexadecimal_upper_chars(detail::static_array_u8_view<N> a) noexcept
    {
        static_array_to_hexadecimal_chars_result<N> r;
        auto* p = detail::static_array_to_hexadecimal_chars_result_access::buffer(r);
        for (uint8_t i : a) {
            p = int_to_fixed_hexadecimal_chars(p, i);
        }
        return r;
    }

    template<std::size_t N>
    static_array_to_hexadecimal_chars_result<N>
    static_array_to_hexadecimal_lower_chars(detail::static_array_u8_view<N> a) noexcept
    {
        static_array_to_hexadecimal_chars_result<N> r;
        auto* p = detail::static_array_to_hexadecimal_chars_result_access::buffer(r);
        for (uint8_t i : a) {
            p = int_to_fixed_hexadecimal_lower_chars(p, i);
        }
        return r;
    }

    template<std::size_t N>
    static_array_to_hexadecimal_zchars_result<N>
    static_array_to_hexadecimal_upper_zchars(detail::static_array_u8_view<N> a) noexcept
    {
        static_array_to_hexadecimal_zchars_result<N> r;
        auto* p = detail::static_array_to_hexadecimal_chars_result_access::buffer(r);
        for (uint8_t i : a) {
            p = int_to_fixed_hexadecimal_chars(p, i);
        }
        *p = '\0';
        return r;
    }

    template<std::size_t N>
    static_array_to_hexadecimal_zchars_result<N>
    static_array_to_hexadecimal_lower_zchars(detail::static_array_u8_view<N> a) noexcept
    {
        static_array_to_hexadecimal_zchars_result<N> r;
        auto* p = detail::static_array_to_hexadecimal_chars_result_access::buffer(r);
        for (uint8_t i : a) {
            p = int_to_fixed_hexadecimal_lower_chars(p, i);
        }
        *p = '\0';
        return r;
    }
}


template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
inline static_array_to_hexadecimal_upper_chars(uint8_t const (&a)[N]) noexcept
{
    return detail::static_array_to_hexadecimal_upper_chars(
        detail::static_array_u8_view<N>{a});
}

template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
inline static_array_to_hexadecimal_lower_chars(uint8_t const (&a)[N]) noexcept
{
    return detail::static_array_to_hexadecimal_lower_chars(
        detail::static_array_u8_view<N>{a});
}

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
inline static_array_to_hexadecimal_upper_zchars(uint8_t const (&a)[N]) noexcept
{
    return detail::static_array_to_hexadecimal_upper_zchars(
        detail::static_array_u8_view<N>{a});
}

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
inline static_array_to_hexadecimal_lower_zchars(uint8_t const (&a)[N]) noexcept
{
    return detail::static_array_to_hexadecimal_lower_zchars(
        detail::static_array_u8_view<N>{a});
}


template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
inline static_array_to_hexadecimal_upper_chars(std::array<uint8_t, N> const& a) noexcept
{
    return detail::static_array_to_hexadecimal_upper_chars(
        detail::static_array_u8_view<N>{a.data()});
}

template<std::size_t N>
static_array_to_hexadecimal_chars_result<N>
inline static_array_to_hexadecimal_lower_chars(std::array<uint8_t, N> const& a) noexcept
{
    return detail::static_array_to_hexadecimal_lower_chars(
        detail::static_array_u8_view<N>{a.data()});
}

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
inline static_array_to_hexadecimal_upper_zchars(std::array<uint8_t, N> const& a) noexcept
{
    return detail::static_array_to_hexadecimal_upper_zchars(
        detail::static_array_u8_view<N>{a.data()});
}

template<std::size_t N>
static_array_to_hexadecimal_zchars_result<N>
inline static_array_to_hexadecimal_lower_zchars(std::array<uint8_t, N> const& a) noexcept
{
    return detail::static_array_to_hexadecimal_lower_zchars(
        detail::static_array_u8_view<N>{a.data()});
}
