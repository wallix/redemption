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
Copyright (C) Wallix 2017
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/rect.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

/**
 * \brief apply f on each splitted sub-rect
 * \param f  fonctor with a Rect parameter

    +-----+-----+-+
    |     |     | |
    |  1  |  2  |3|
    |     |     | |
    +-----+-----+-+
    |  4  |  5  |6|
    +-----+-----+-┘
*/

template<class F>
void contiguous_sub_rect_f(
    checked_int<uint16_t> const cx_,
    checked_int<uint16_t> const cy_,
    checked_int<uint16_t> const sub_cx_cy_,
    F && f)
{
    uint16_t const cx = cx_;
    uint16_t const cy = cy_;
    uint16_t const sub_cx_cy = sub_cx_cy_;

    Rect rect;

    for (rect.y = 0; rect.y < cy; rect.y += sub_cx_cy) {
        rect.cy = std::min(sub_cx_cy, uint16_t(cy - rect.y));

        for (rect.x = 0; rect.x < cx ; rect.x += sub_cx_cy) {
            rect.cx = std::min(sub_cx_cy, uint16_t(cx - rect.x));
            f(const_cast<Rect const&>(rect));
        }
    }
}
