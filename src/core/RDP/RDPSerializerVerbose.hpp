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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "utils/verbose_flags.hpp"


REDEMPTION_VERBOSE_FLAGS_DEF(RDPSerializerVerbose)
{
    none,
    pointer             = 0x0004,
    primary_orders      = 0x0020,
    secondary_orders    = 0x0040,
    bitmap_update       = 0x0080,
    surface_commands    = 0x0100,
    bmp_cache           = 0x0200,
    internal_buffer     = 0x0400,
    sec_decrypted       = 0x1000,
    // update Front::Verbose when verbose > 0xffff
};
