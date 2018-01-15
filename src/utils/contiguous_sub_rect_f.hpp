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

struct CxCy
{
    explicit CxCy(checked_int<uint16_t> cx, checked_int<uint16_t> cy) noexcept
      : cx(cx)
      , cy(cy)
    {}

    uint16_t const cx;
    uint16_t const cy;
};

struct SubCxCy
{
    explicit SubCxCy(checked_int<uint16_t> cx, checked_int<uint16_t> cy) noexcept
      : cx(cx)
      , cy(cy)
    {}

    uint16_t const cx;
    uint16_t const cy;
};

/**
 * \brief apply f on each splitted sub-rect with maximal size specified by \c sub_wh
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
void contiguous_sub_rect_f(CxCy wh, SubCxCy sub_wh, F && f)
{
    Rect rect;

    for (rect.y = 0; rect.y < wh.cy; rect.y += sub_wh.cy) {
        rect.cy = std::min(sub_wh.cy, uint16_t(wh.cy - rect.y));

        for (rect.x = 0; rect.x < wh.cx ; rect.x += sub_wh.cx) {
            rect.cx = std::min(sub_wh.cx, uint16_t(wh.cx - rect.x));
            f(const_cast<Rect const&>(rect));
        }
    }
}

// template<class F>
// void optimal_sub_rect_f(CxCy wh, std::size_t sub_pixcount, F && f);
