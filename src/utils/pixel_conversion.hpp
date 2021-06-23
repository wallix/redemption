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
*/

#pragma once

#include "utils/colors.hpp"


namespace pixel_conversion_fns
{
    constexpr auto buf2col_1B
      = [](uint8_t const * p) { return RDPColor::from(p[0]); };
    constexpr auto buf2col_2B
      = [](uint8_t const * p) { return RDPColor::from(uint32_t(p[0] | (p[1] << 8))); };
    constexpr auto buf2col_3B
      = [](uint8_t const * p) { return RDPColor::from(uint32_t(p[0] | (p[1] << 8) | (p[2] << 16))); };
    constexpr auto buf2col_4B = buf2col_3B;

    constexpr auto col2buf_1B
      = [](RDPColor c, uint8_t * p) {                   p[0] = c.as_bgr().red(); };
    constexpr auto col2buf_2B
      = [](RDPColor c, uint8_t * p) { col2buf_1B(c, p); p[1] = c.as_bgr().green(); };
    constexpr auto col2buf_3B
      = [](RDPColor c, uint8_t * p) { col2buf_2B(c, p); p[2] = c.as_bgr().blue(); };
    constexpr auto col2buf_4B
      = [](RDPColor c, uint8_t * p) { col2buf_2B(c, p); p[2] = c.as_bgr().blue(); p[3] = 0xff; };
} // namespace pixel_conversion_fns
