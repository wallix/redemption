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

#include "utils/static_string.hpp"
#include "utils/sugar/int_to_chars.hpp"


namespace detail
{
    struct static_fmt_part
    {
        uint16_t offset;
        uint16_t length;
        char fmt;
    };

    struct static_fmt_init_t
    {
        static_fmt_part* part;
        char* string;
        char* s1 = string;
        char* s2 = string;
        bool is_fmt = false;
        int i = 0;
        char fmt_err = 0;
        int idx_err = 0;

        constexpr bool next(char c)
        {
            if (is_fmt) {
                if (c == 's' || c == 'd' || c == 'u' || c == 'x' || c == 'X') {
                    *part++ = {uint16_t(s1-string), uint16_t(s2-s1), c};
                    s1 = s2;
                }
                else if (c == '%') {
                    *s2++ = '%';
                }
                else {
                    fmt_err = c;
                    idx_err = i;
                    return false;
                }
                is_fmt = false;
            }
            else if (c == '%') {
                is_fmt = true;
            }
            else {
                *s2++ = c;
            }

            ++i;
            return true;
        }

        constexpr bool post_error(bool has_error)
        {
            if (!has_error && is_fmt) {
                has_error = true;
                idx_err = i;
                fmt_err = 0;
                return true;
            }
            return false;
        }

        constexpr static_fmt_part post()
        {
            return {uint16_t(s1-string), uint16_t(s2-s1), '\0'};
        }
    };

    template<char... cs>
    constexpr auto make_static_fmt()
    {
        struct fmt_t
        {
            detail::static_fmt_part parts[(... + (cs == '%'))];
            detail::static_fmt_part last;
            char string[sizeof...(cs)];
            char fmt_err;
            int idx_err;
            std::size_t string_len;
            std::size_t part_count;
            bool has_error;
        };

        fmt_t fmt {};

        detail::static_fmt_init_t fmt_init{fmt.parts, fmt.string};
        fmt.has_error = (... && fmt_init.next(cs));
        fmt.has_error = fmt_init.post_error(fmt.has_error);
        fmt.last = fmt_init.post();
        fmt.fmt_err = fmt_init.fmt_err;
        fmt.idx_err = fmt_init.idx_err;
        fmt.string_len = std::size_t(fmt_init.s2 - fmt.string);
        fmt.part_count = std::size_t(fmt_init.part - fmt.parts);

        return fmt;
    }

    struct static_fmt_no_error {};

    template<std::size_t Offset, std::size_t FmtCount, char Fmt>
    struct static_fmt_error {};

    template<uint16_t Offset, uint16_t Length, char Fmt>
    struct static_fmt_part_t
    {
        static constexpr std::uint16_t offset = Offset;
        static constexpr std::uint16_t length = Length;
        static constexpr char fmt = Fmt;
    };

    template<char fmt>
    struct static_fmt_x_convertor
    {};

    template<>
    struct static_fmt_x_convertor<'s'>
    {
        template<class T>
        static auto convert(T const& x)
        {
            if constexpr (std::is_integral_v<T>) {
                return int_to_decimal_chars(x);
            }
            else {
                return to_static_string_view_or_char(x);
            }
        }
    };

    template<>
    struct static_fmt_x_convertor<'d'>
    {
        template<class T>
        static int_to_chars_result convert(T x) noexcept
        {
            static_assert(std::is_signed_v<T>);
            return int_to_decimal_chars(x);
        }
    };

    template<>
    struct static_fmt_x_convertor<'u'>
    {
        template<class T>
        static int_to_chars_result convert(T x) noexcept
        {
            static_assert(std::is_unsigned_v<T>);
            return int_to_decimal_chars(x);
        }
    };

    template<>
    struct static_fmt_x_convertor<'x'>
    {
        template<class T>
        static int_to_chars_result convert(T x) noexcept
        {
            return int_to_hexadecimal_lower_chars(x);
        }
    };

    template<>
    struct static_fmt_x_convertor<'X'>
    {
        template<class T>
        static int_to_chars_result convert(T x) noexcept
        {
            return int_to_hexadecimal_upper_chars(x);
        }
    };

    template<std::size_t MaxSize, class BufFormat, class LastPart, class... Parts>
    struct static_fmt_t
    {
        template<class... Ts>
        auto operator()(Ts const&... xs) const
        {
            static_assert(sizeof...(Parts) == sizeof...(Ts));
            return format_impl(to_static_string_view_or_char(
                static_fmt_x_convertor<Parts::fmt>::convert(xs)
            )...);
        }

        template<std::size_t n, class... Ts>
        void write_to(static_string<n>& str, Ts const&... xs) const
        {
            static_assert(sizeof...(Parts) == sizeof...(Ts));
            write_to_impl(
                str,
                to_static_string_view_or_char(
                    static_fmt_x_convertor<Parts::fmt>::convert(xs)
                )...
            );
        }

        template<std::size_t NewMaxSize>
        constexpr static_fmt_t<NewMaxSize, BufFormat, LastPart, Parts...>
        set_max_size() noexcept
        {
            return {};
        }

    private:
        template<class... Strs>
        static auto format_impl(Strs const&... strs)
        {
            constexpr auto max_size
                = (std::size_t() + ... + Parts::length)
                + (std::size_t() + ... + static_str_len<Strs>::value)
                + LastPart::length;

            static_assert(max_size <= MaxSize);

            static_string<max_size> str;
            impl(str, strs...);
            return str;
        }

        template<std::size_t n, class... Strs>
        static void write_to_impl(static_string<n>& str, Strs const&... strs)
        {
            constexpr auto max_size
                = (std::size_t() + ... + Parts::length)
                + (std::size_t() + ... + static_str_len<Strs>::value)
                + LastPart::length;

            static_assert(max_size <= n);
            impl(str, strs...);
        }

        template<std::size_t n, class... Strs>
        static void impl(static_string<n>& str, Strs const&... strs)
        {
            char* p = str.data();
            char* e = p;

            (..., void(e = append_from_bounded_av_or_char(
                append_from_bounded_av_or_char(e,
                    sized_chars_view<Parts::length>
                    ::assumed(BufFormat::data + Parts::offset)
                ),
                to_static_string_view_or_char(strs)
            )));

            e = append_from_bounded_av_or_char(e,
                sized_chars_view<LastPart::length>
                    ::assumed(BufFormat::data + LastPart::offset)
                );

            *e = '\0';

            auto len = std::size_t(e-p);
            static_string_set_size::set_size(str, len);
        }
    };
} // namespace detail

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wpedantic")
template<class C, C... cs>
constexpr auto operator "" _static_fmt() noexcept
{
    // static_assert(std::is_same_v<C, char>);
    static_assert(sizeof...(cs) >= 2);
    static_assert(sizeof...(cs) < unsigned(~uint16_t()));

    constexpr auto fmt = detail::make_static_fmt<cs...>();

    if constexpr (fmt.has_error) {
        // readable compiler error
        detail::static_fmt_no_error() = detail::static_fmt_error<
            fmt.idx_err, fmt.part_count+1, fmt.fmt_err
        >();
        return []([[maybe_unused]] auto... xs) { return static_string<0>(); };
    }
    else {
        auto to_static_str = [&](auto... ints) {
            return detail::static_constexpr_array<fmt.string[ints]...>();
        };
        using static_str = decltype(detail::unroll_indexes(
            std::make_index_sequence<fmt.string_len>(),
            to_static_str));

        auto to_static_fmt = [&](auto... ints) {
            return detail::static_fmt_t<
                4096,
                static_str,
                detail::static_fmt_part_t<
                    fmt.last.offset,
                    fmt.last.length,
                    fmt.last.fmt
                >,
                detail::static_fmt_part_t<
                    fmt.parts[ints].offset,
                    fmt.parts[ints].length,
                    fmt.parts[ints].fmt
                >...
            >();
        };

        return detail::unroll_indexes(
            std::make_index_sequence<fmt.part_count>(),
            to_static_fmt);
    }
}
REDEMPTION_DIAGNOSTIC_POP()
