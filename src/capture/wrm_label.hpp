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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/


#pragma once

// TODO enum class
enum {
    META_FILE           = 1006,
    TIMESTAMP           = 1008,
    POINTER             = 1009,
    POINTER2            = 1010,
    LAST_IMAGE_CHUNK    = 0x1000,   // 4096
    PARTIAL_IMAGE_CHUNK = 0x1001,   // 4097
    SAVE_STATE          = 0x1002,   // 4098
    RESET_CHUNK         = 0x1003,   // 4099
    SESSION_UPDATE      = 0x1004,

    INVALID_CHUNK       = 0x8000
};

