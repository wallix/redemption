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

#include "utils/cli.hpp"
#include "gdi/screen_info.hpp"

namespace cli::parsers
{
    template<>
    struct argname_value_traits<BitsPerPixel>
    {
        static constexpr std::string_view default_argname = "<8|15|16|24|32>";
    };

    template<>
    struct argname_value_traits<ScreenInfo>
    {
        static constexpr std::string_view default_argname = "<{width}x{height} | {width}x{height}x{bpp}>";
    };
}

namespace cli::arg_parsers
{
    template<>
    struct arg_parse_traits<BitsPerPixel>
    {
        using value_type = BitsPerPixel;

        static Res parse(value_type& out, char const* str)
        {
            uint8_t n;

            Res res = detail::arg_parse_int(n, str);
            if (res != Res::Ok) {
                return res;
            }

            switch (n) {
            case 8:
            case 15:
            case 16:
            case 24:
            case 32:
                out = checked_int(n);
                return cli::Res::Ok;
            default:
                return cli::Res::BadFormat;
            }
        }
    };

    template<>
    struct arg_parse_traits<ScreenInfo>
    {
        using value_type = ScreenInfo;

        static Res parse(value_type& out, char const* str)
        {
            auto rw = decimal_chars_to_int<uint16_t>(str);
            if (rw.ec == std::errc() && *rw.ptr == 'x') {
                auto rh = decimal_chars_to_int<uint16_t>(rw.ptr+1);
                if (rh.ec != std::errc()) {
                    return cli::Res::BadFormat;
                }
                if (*rh.ptr) {
                    arg_parse_traits<BitsPerPixel>().parse(out.bpp, rh.ptr);
                }
                out.width = rw.val;
                out.height = rh.val;
                return cli::Res::Ok;
            }

            return cli::Res::BadFormat;
        }
    };
} // namespace cli::arg_parsers
