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
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once

#include <cstdint>

enum class WrmChunkType : uint16_t
{
    RDP_UPDATE_ORDERS   = 0,        // ::RDP_UPDATE_ORDERS
    RDP_UPDATE_BITMAP   = 1,        // ::RDP_UPDATE_BITMAP (with bug on compression)
    RDP_UPDATE_BITMAP2  = 0x3ED,    // ::RDP_UPDATE_BITMAP

    META_FILE           = 0x3EE,    // 1006
    // formely TIMESTAMP on real time,
    // now a monotonic time that starts at 0.
    // real time is in TIMES
    TIMESTAMP_OR_RECORD_DELAY = 0x3F0,    // 1008
    POINTER                   = 0x3F1,    // 1009
    POINTER2                  = 0x3F2,    // 1010
    POINTER_NATIVE            = 0x3F3,    // 1011
    TIMES                     = 0x3F4,    // 1012

    LAST_IMAGE_CHUNK          = 0x1000,   // 4096
    PARTIAL_IMAGE_CHUNK       = 0x1001,   // 4097
    SAVE_STATE                = 0x1002,   // 4098
    RESET_CHUNK               = 0x1003,   // 4099
    OLD_SESSION_UPDATE        = 0x1004,
    SESSION_UPDATE            = 0x1005,

    POSSIBLE_ACTIVE_WINDOW_CHANGE = 0x2000,
    IMAGE_FRAME_RECT              = 0x2001,
    KBD_INPUT_MASK                = 0x2002,
    MONITOR_LAYOUT                = 0x2003,
    RAIL_WINDOW_RECT_START        = 0x2004,

    // 0x5000 ~ 0x5FFF : Reserved for SSH Proxy.

    INVALID_CHUNK       = 0x8000
};
