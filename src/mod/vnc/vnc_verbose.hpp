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
    Copyright (C) Wallix 2018
    Author(s): Christophe Grosjean
*/

#pragma once

#include "utils/verbose_flags.hpp"

REDEMPTION_VERBOSE_FLAGS_DEF(VNCVerbose)
{
    none,

    basic_trace     = 0x00000001,
    keymap_stack    = 0x00000002,
    draw_event      = 0x00000004,
    input           = 0x00000008,
    connection      = 0x00000010,
    hextile_encoder = 0x00000020,
    cursor_encoder  = 0x00000040,
    clipboard       = 0x00000080,
    zrle_encoder    = 0x00000100,
    zrle_trace      = 0x00000200,
    hextile_trace   = 0x00000400,
    cursor_trace    = 0x00001000,
    rre_encoder     = 0x00002000,
    rre_trace       = 0x00004000,
    raw_encoder     = 0x00008000,
    raw_trace       = 0x00010000,
    copyrect_encoder= 0x00020000,
    copyrect_trace  = 0x00040000,

    keymap          = 0x00080000,
};

