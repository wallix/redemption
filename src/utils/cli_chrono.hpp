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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#pragma once

#include <chrono>

#include "utils/cli.hpp"

namespace cli::detail
{
    template<class Period>
    struct argname_period_traits : parsers::argname_value_traits<void>
    {};

    template<>
    struct argname_period_traits<std::micro>
    {
        static constexpr std::string_view default_argname = "<microseconds>";
    };

    template<>
    struct argname_period_traits<std::milli>
    {
        static constexpr std::string_view default_argname = "<milliseconds>";
    };

    template<>
    struct argname_period_traits<std::ratio<1>>
    {
        static constexpr std::string_view default_argname = "<seconds>";
    };

    template<>
    struct argname_period_traits<std::ratio<60>>
    {
        static constexpr std::string_view default_argname = "<minutes>";
    };

    template<>
    struct argname_period_traits<std::ratio<3600>>
    {
        static constexpr std::string_view default_argname = "<hours>";
    };
} // namespace cli::detail

namespace cli::parsers
{
    template<class Rep, class Period>
    struct argname_value_traits<std::chrono::duration<Rep, Period>>
    : ::cli::detail::argname_period_traits<Period>
    {};
}

namespace cli::arg_parsers
{
    template<class Rep, class Period>
    struct arg_parse_traits<std::chrono::duration<Rep, Period>>
    {
        using value_type = std::chrono::duration<Rep, Period>;

        static Res parse(value_type& result, char const* s)
        {
            auto r = decimal_chars_to_int<Rep>(s);

            if (r.ec == std::errc()) {
                if (!*r.ptr) {
                    result = value_type(r.val);
                    return Res::Ok;
                }

                // format MM:SS / HH:MM:SS with seconds
                if constexpr (std::is_same_v<std::chrono::seconds, std::chrono::duration<Rep, Period>>) {
                    if (*r.ptr == ':') {
                        auto r2 = decimal_chars_to_int<Rep>(r.ptr + 1);
                        if (r2.ec == std::errc()) {
                            if (!*r2.ptr) {
                                result = value_type(r.val * 60 + r2.val);
                                return Res::Ok;
                            }

                            if (*r2.ptr == ':') {
                                auto r3 = decimal_chars_to_int<Rep>(r2.ptr + 1);
                                if (r3.ec == std::errc() && !*r3.ptr) {
                                    result = value_type(r.val * 3600 + r2.val * 60 + r3.val);
                                    return Res::Ok;
                                }
                            }
                        }
                    }
                }
            }

            return Res::BadValueFormat;
        }
    };
} // namespace cli::arg_parsers
