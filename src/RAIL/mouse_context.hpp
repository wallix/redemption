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
#include "core/RDP/rdp_pointer.hpp"

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

//                          corner
//                        |-------|
//                  cx
//    |---------------------------|
//        1         0        11
//    +--NWN--\-----N-----|--NEN--+      +   +
//    |                           |      |   |
// 2 NWW                         NEE 10  |   | corner
//    |                           |      |   |
//    +         N = North         +      |   +
//    |         S = South         |      |
//    |         E = East          |      |
// 3  W         W = West          E 9    | cy
//    |                           |      |
//    |                           |      |
//    +                           +      |
//    |                           |      |
// 4 SWW                         SEE 8   |
//    |                           |      |
//    +--SWS--\-----S-----|--SES--+      °
//        5         6         7

enum { ZONE_N, ZONE_NWN, ZONE_NWW, ZONE_W, ZONE_SWW, ZONE_SWS, ZONE_S, ZONE_SES, ZONE_SEE, ZONE_E, ZONE_NEE, ZONE_NEN };


static inline int get_button(size_t zone)
{
    switch (zone){
    case ZONE_N  : return MouseContext::MOUSE_BUTTON_PRESSED_NORTH;
    case ZONE_NWN: return MouseContext::MOUSE_BUTTON_PRESSED_NORTHWEST;
    case ZONE_NWW: return MouseContext::MOUSE_BUTTON_PRESSED_NORTHWEST;
    case ZONE_W  : return MouseContext::MOUSE_BUTTON_PRESSED_WEST;
    case ZONE_SWW: return MouseContext::MOUSE_BUTTON_PRESSED_SOUTHWEST;
    case ZONE_SWS: return MouseContext::MOUSE_BUTTON_PRESSED_SOUTHWEST;
    case ZONE_S  : return MouseContext::MOUSE_BUTTON_PRESSED_SOUTH;
    case ZONE_SES: return MouseContext::MOUSE_BUTTON_PRESSED_SOUTHEAST;
    case ZONE_SEE: return MouseContext::MOUSE_BUTTON_PRESSED_SOUTHEAST;
    case ZONE_E  : return MouseContext::MOUSE_BUTTON_PRESSED_EAST;
    case ZONE_NEE: return MouseContext::MOUSE_BUTTON_PRESSED_NORTHEAST;
    case ZONE_NEN: return MouseContext::MOUSE_BUTTON_PRESSED_NORTHEAST;
    }
}

static inline int get_pointer_type(size_t zone)
{
    switch (zone){
    case ZONE_N  : return Pointer::POINTER_SIZENS;
    case ZONE_NWN: return Pointer::POINTER_SIZENWSE;
    case ZONE_NWW: return Pointer::POINTER_SIZENWSE;
    case ZONE_W  : return Pointer::POINTER_SIZEWE;
    case ZONE_SWW: return Pointer::POINTER_SIZENESW;
    case ZONE_SWS: return Pointer::POINTER_SIZENESW;
    case ZONE_S  : return Pointer::POINTER_SIZENS;
    case ZONE_SES: return Pointer::POINTER_SIZENWSE;
    case ZONE_SEE: return Pointer::POINTER_SIZENWSE;
    case ZONE_E  : return Pointer::POINTER_SIZEWE;
    case ZONE_NEE: return Pointer::POINTER_SIZENESW;
    case ZONE_NEN: return Pointer::POINTER_SIZENESW;
    }
}

static inline Pointer get_pointer(size_t zone)
{
    switch (zone){
    case ZONE_N  : return size_NS_pointer();
    case ZONE_NWN: return size_NESW_pointer();
    case ZONE_NWW: return size_NESW_pointer();
    case ZONE_W  : return size_WE_pointer();
    case ZONE_SWW: return size_NESW_pointer();
    case ZONE_SWS: return size_NESW_pointer();
    case ZONE_S  : return size_NS_pointer();
    case ZONE_SES: return size_NESW_pointer();
    case ZONE_SEE: return size_NESW_pointer();
    case ZONE_E  : return size_WE_pointer();
    case ZONE_NEE: return size_NESW_pointer();
    case ZONE_NEN: return size_NESW_pointer();
    }
}


static inline Rect get_zone(size_t zone, Rect w, uint16_t corner, uint16_t thickness)
{

    uint8_t data[12][4] ={
    // North
    { 1, 0, 0},

    // North West North
    { 0, 0, 0},
    // North West West
    { 0, 0, 1},

    // West
    { 0, 1, 1},

    // South West West
    { 0, 2, 1},
    // South West South
    { 0, 2, 0},

    // South
    { 1, 2, 0},

    // South East South
    { 2, 2, 0},
    // South East East
    { 2, 2, 1},

    // East
    { 2, 1, 1},

    // North East East
    { 2, 0, 1},
    // North East North
    { 2, 0, 0},
    };

    // d[0] 0=left or 1=middle, 2=right
    // d[1] 0=top or 1=middle or 2=bottom
    // d[2] 0=horizontal 1=vectical

    auto & d = data[zone];

    return Rect(
        w.x + ((d[0]==0)?0:(d[0]==1)?corner:(w.cx-((d[2]==0)?corner:thickness))),
        w.y + ((d[1]==0)?0:(d[1]==1)?corner:(w.cy-((d[2]==1)?corner:thickness))),
        (d[0]==1)?w.cx-2*corner:(d[2]==0)?corner:thickness,
        (d[1]==1)?w.cy-2*corner:(d[2]==1)?corner:thickness
    );
}

