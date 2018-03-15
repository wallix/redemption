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
    basic_trace     = 0x0001,
    keymap_stack    = 0x0002,
    draw_event      = 0x0004,
    input           = 0x0008,
    connection      = 0x0010,
    hextile_encoder = 0x0020,
    cursor_encoder  = 0x0040,

    clipboard       = 0x0080,
};

