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

#include "utils/image_view.hpp"
#include "utils/sugar/noncopyable.hpp"


class RdpPointerView;
class Drawable;

class DrawablePointer : noncopyable
{
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
    };

    uint16_t cursor_x = 0;
    uint16_t cursor_y = 0;
    uint16_t hotspot_x = 0;
    uint16_t hotspot_y = 0;

    uint16_t width = 0;
    uint16_t height = 0;

    uint8_t data[MAX_WIDTH * MAX_HEIGHT * 4]; // 96 pixels per line * 96 lines * 4 bytes per pixel
    uint8_t mask[MAX_WIDTH * MAX_HEIGHT * 3]; // 96 pixels per line * 96 lines * 3 bytes per pixel

    ImageView image_data_view_data;
    ImageView image_data_view_mask;

public:
    using BufferSaver = uint8_t[MAX_WIDTH * MAX_HEIGHT * 3];

    DrawablePointer();

    DrawablePointer(RdpPointerView const& cursor);

    void set_cursor(RdpPointerView const& cursor);

    void set_position(uint16_t x, uint16_t y)
    {
        this->cursor_x = x;
        this->cursor_y = y;
    }

    void trace_mouse(Drawable& drawable, BufferSaver& drawable_buffer) const;
    void clear_mouse(Drawable& drawable, BufferSaver const& drawable_buffer) const;
};  // struct DrawablePointer
