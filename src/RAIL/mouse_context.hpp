/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "utils/log.hpp"
#include "utils/rect.hpp"
#include "core/error.hpp"
#include "core/RDP/remote_programs.hpp"

enum { PTRFLAGS_EX_DOUBLE_CLICK = 0xFFFF };
enum {BORDER_WIDTH_HEIGHT = 3 };

struct MouseContext {
    enum {
        MOUSE_BUTTON_PRESSED_NONE,

        MOUSE_BUTTON_PRESSED_NORTH,
        MOUSE_BUTTON_PRESSED_NORTHWEST,
        MOUSE_BUTTON_PRESSED_WEST,
        MOUSE_BUTTON_PRESSED_SOUTHWEST,
        MOUSE_BUTTON_PRESSED_SOUTH,
        MOUSE_BUTTON_PRESSED_SOUTHEAST,
        MOUSE_BUTTON_PRESSED_EAST,
        MOUSE_BUTTON_PRESSED_NORTHEAST,

        MOUSE_BUTTON_PRESSED_TITLEBAR,
        MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX,
        MOUSE_BUTTON_PRESSED_MINIMIZEBOX,
        MOUSE_BUTTON_PRESSED_MAXIMIZEBOX,
        MOUSE_BUTTON_PRESSED_CLOSEBOX,
    };

    int window_offset_x = 0;
    int window_offset_y = 0;

    Rect virtual_screen_rect;

    Rect task_bar_rect;

    Rect window_rect;
    Rect window_rect_saved;
    Rect window_rect_normal;
    Rect window_rect_old;

    Rect title_bar_icon_rect;
    Rect title_bar_rect;
    Rect close_box_rect;
    Rect minimize_box_rect;
    Rect maximize_box_rect;
    Rect resize_hosted_desktop_box_rect;

    Rect north;
    Rect north_west_north;
    Rect north_west_west;
    Rect west;
    Rect south_west_west;
    Rect south_west_south;
    Rect south;
    Rect south_east_south;
    Rect south_east_east;
    Rect east;
    Rect north_east_east;
    Rect north_east_north;

    int button_1_down_timer;

    int button_1_down_x = 0;
    int button_1_down_y = 0;

    int button_1_down = MouseContext::MOUSE_BUTTON_PRESSED_NONE;

    uint16_t captured_mouse_x = 0;
    uint16_t captured_mouse_y = 0;

    int pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;
    bool move_size_initialized = false;
    bool verbose = false;

    void update_rects(const bool allow_resize_hosted_desktop);
    Rect get_window_rect() const;
    Point get_window_offset() const;

};

