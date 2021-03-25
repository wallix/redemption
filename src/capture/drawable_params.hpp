/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean
*/

#pragma once

#include <cstdint>

class RDPDrawable;

struct DrawableParams
{
    // width, height and no_mouse are ignored when rdp_drawable != nullptr
    uint16_t width;
    uint16_t height;
    bool no_mouse;

    RDPDrawable* rdp_drawable;

    static DrawableParams delayed_drawable(uint16_t width, uint16_t height, bool no_mouse)
    {
        return {width, height, no_mouse, nullptr};
    }

    static DrawableParams shared_drawable(RDPDrawable&& rdp_drawable) = delete;
    static DrawableParams shared_drawable(RDPDrawable& rdp_drawable)
    {
        return {0, 0, false, &rdp_drawable};
    }
};
