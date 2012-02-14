/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   RDP Capabilities : 

*/

#if !defined(__RDP_CAPABILITIES_SOUND_HPP__)
#define __RDP_CAPABILITIES_SOUND_HPP__

#include "constants.hpp"

static inline void out_sound_caps(Stream & stream)
{
    const char caps_sound[] = { 0x01, 0x00, 0x00, 0x00 };
    stream.out_uint16_le(0x0C);
    stream.out_uint16_le(8);
    stream.out_copy_bytes(caps_sound, 4);
}

#endif

