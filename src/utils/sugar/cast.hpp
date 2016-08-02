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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

    simple usual casts

*/

#pragma once
#include <stdint.h>

static inline uint8_t * byte_ptr_cast(char * data)
{
    return reinterpret_cast<uint8_t *>(data);
}

static inline const uint8_t * byte_ptr_cast(const char * data)
{
    return reinterpret_cast<const uint8_t *>(data);
}

static inline char * char_ptr_cast(uint8_t * data)
{
    return reinterpret_cast<char *>(data);
}

static inline const char * char_ptr_cast(const uint8_t * data)
{
    return reinterpret_cast<const char *>(data);
}
