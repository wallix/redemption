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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Conversion function between packed and memory aligned memory bitmap format

*/

#ifndef ALTOCO__HPP
#define ALTOCO__HPP

#include <stdint.h>
#include <assert.h>

static inline uint16_t align4(int value)
{
    return (value+3) & ~3;
}

static inline uint8_t nbbytes(unsigned value)
{
    return (uint8_t)((value+7) / 8);
}

static inline void out_bytes_le(uint8_t * ptr, const uint8_t nb, const unsigned value)
{
    for (int b = 0 ; b < nb ; ++b){
        ptr[b] = value >> (8 * b);
    }
}

static inline unsigned in_bytes_le(const uint8_t nb, const uint8_t * ptr)
{
    unsigned res = 0;
    for (int b = 0 ; b < nb ; ++b){
        res |= ptr[b] << (8 * b);
    }
    return res;
}

static inline uint16_t row_size(uint16_t width, uint8_t bpp)
{
    return align4(width * nbbytes(bpp));
}

#endif
