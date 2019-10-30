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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/rect.hpp"

#include <array>


namespace gdi
{

using subrect4_t = std::array<Rect, 4>;

inline subrect4_t subrect4(const Rect rect, const Rect & exclude_rect)
{
    const Rect inter = rect.intersect(exclude_rect);
    return {{
        // top
        Rect(rect.x, rect.y, rect.cx, inter.y - rect.y),
        // right
        Rect(inter.eright(), inter.y, rect.eright() - inter.eright(), inter.cy),
        // bottom
        Rect(rect.x, inter.ebottom(), rect.cx, rect.ebottom() - inter.ebottom()),
        // left
        Rect(rect.x, inter.y, inter.x - rect.x, inter.cy)
    }};
}

} // namespace gdi
