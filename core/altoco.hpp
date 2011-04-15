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

static inline uint32_t align4(int value)
{
    return ((value+3) & (-1^3));
}

static inline uint32_t nbbytes(unsigned value)
{
    assert(value); // we don't want to use it for 0 as it does not work for it
    return ((value+7) / 8);
}

#warning change that to aligned_row_size
static inline uint32_t aligned_size(int width, int height, uint8_t bpp)
{
    return align4(width) * height * ((bpp<=8)?1:(bpp<=16)?2:4);
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
    switch (nb){
    case 3:
        res = (ptr[0] << 16)|(ptr[1] << 8)|ptr[2];
    break;
    case 2:
        res = (ptr[0] << 8)|ptr[1];
    break;
    case 1:
        res = ptr[0];
    break;
    }
    return res;
}


static inline uint32_t row_size(int width, uint8_t bpp)
{
    return align4(width * nbbytes(bpp));
}

static void compact_to_aligned(int width, int height, uint8_t bpp, uint8_t const * const compact, uint8_t * aligned)
{
    int arow = aligned_size(width, 1, bpp);
    int crow = row_size(width, bpp);

    switch (bpp){
    case 8: case 15: case 16:
        for (int y = 0 ; y < height ; y++){
            memcpy(aligned + y * arow, compact + y * crow, crow);
            if (arow > crow){
                memset(aligned + y * arow + crow, 0, arow - crow);
            }
        }
    break;
    case 24: default:
        for (int y = 0 ; y < height ; y++){
            for (int x = 0 ; x < width ; x++){
                0[aligned + y*arow + x * 4] = 0[compact + y*crow + x * 3];
                1[aligned + y*arow + x * 4] = 1[compact + y*crow + x * 3];
                2[aligned + y*arow + x * 4] = 2[compact + y*crow + x * 3];
                3[aligned + y*arow + x * 4] = 0;
            }
        }
    }
}

//static void aligned_to_compact(int width, int height, uint8_t bpp, uint8_t const * const aligned, uint8_t * compact)
//{
//    int arow = aligned_size(width, 1, bpp);
//    int crow = compact_size(width, 1, bpp);

//    switch (bpp){
//    case 8: case 15: case 16:
//        for (int y = 0 ; y < height ; y++){
//            memcpy(compact + y * crow, aligned + y * arow, crow);
//        }
//    break;
//    case 24: default:
//        for (int y = 0 ; y < height ; y++){
//            for (int x = 0 ; x < width ; x++){
//                0[compact + y*crow + x * 3] = 0[aligned + y*arow + x * 4];
//                1[compact + y*crow + x * 3] = 1[aligned + y*arow + x * 4];
//                2[compact + y*crow + x * 3] = 2[aligned + y*arow + x * 4];
//            }
//        }
//    }
//}

#endif
