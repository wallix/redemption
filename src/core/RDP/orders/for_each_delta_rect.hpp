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

#include "utils/rect.hpp"

template<class RDPMulti, class FRect>
inline void for_each_delta_rect(const RDPMulti & cmd, FRect f)
{
    Rect cmd_rect;

    // u8 to int for inhibit overflow and infinite loop
    const int n = cmd.nDeltaEntries;
    for (int i = 0; i < n; i++) {
        cmd_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
        cmd_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
        cmd_rect.cx =  cmd.deltaEncodedRectangles[i].width;
        cmd_rect.cy =  cmd.deltaEncodedRectangles[i].height;
        f(static_cast<Rect const&>(cmd_rect));
    }
}
