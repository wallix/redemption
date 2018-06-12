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
   Author(s): Christophe Grosjean, Raphael ZHOU
*/

#pragma once

enum {
    WS_DISABLED = 0x08000000L,
    WS_ICONIC   = 0x20000000L,  // The window is initially minimized. Same as the WS_MINIMIZE style.
    WS_MINIMIZE = WS_ICONIC,
    WS_SYSMENU  = 0x00080000L,
    WS_VISIBLE  = 0x10000000L   // The window is initially visible.
};

enum {
    SW_FORCEMINIMIZE = 11,
    SW_HIDE          = 0,
    SW_MINIMIZE      = 6
};
