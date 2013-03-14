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

#ifndef _REDEMPTION_UTILS_BITFU_HPP_
#define _REDEMPTION_UTILS_BITFU_HPP_

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

static inline uint32_t nbbytes_large(unsigned value)
{
    return ((value+7) / 8);
}

static inline void out_bytes_le(uint8_t * ptr, const uint8_t nb, const unsigned value)
{
    for (uint8_t b = 0 ; b < nb ; ++b){
        ptr[b] = (uint8_t)(value >> (8 * b));
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

static inline unsigned in_bytes_be(const uint8_t nb, const uint8_t * ptr)
{
    unsigned res = 0;
    for (int b = 0 ; b < nb ; ++b){
        res = (res << 8) | ptr[b];
    }
    return res;
}

static inline uint16_t row_size(uint16_t width, uint8_t bpp)
{
    return align4(width * nbbytes(bpp));
}

// The  rmemcpy() function copies n bytes from memory area src to memory area dest inverting end and beginning.
// The memory areas must not overlap.  Use rmemmove() if the memory areas do overlap.
static inline void rmemcpy(uint8_t *dest, const uint8_t *src, size_t n)
{
    for (size_t i = 0; i < n ; i++){
        dest[n-1-i] = src[i];
    }
}

static inline void reverseit(uint8_t *buffer, size_t n){
    for (size_t i = 0 ; i < n / 2; i++){
        uint8_t tmp = buffer[n-1-i];
        buffer[n-1-i] = buffer[i];
        buffer[i] = tmp;
    }
}

static inline void rmemmove(uint8_t *dest, uint8_t *src, size_t n)
{
    if (src > dest){
        size_t intersect = src - dest;
        if (intersect > n){
            rmemcpy(dest, src, n);
        }
        else {
            rmemcpy(dest, src + intersect, n - intersect);
            reverseit(src, intersect);
        }
    }
    else {
        size_t intersect = dest - src;
        if (intersect > n){
            rmemcpy(dest, src, n);
        }
        else {
            rmemcpy(dest + intersect, src, n - intersect);
            reverseit(dest, intersect);
        }

    }
}

// Output a uint32 into a buffer (little-endian)
static inline void buf_out_uint32(uint8_t* buffer, int value)
{
  buffer[0] = value & 0xff;
  buffer[1] = (value >> 8) & 0xff;
  buffer[2] = (value >> 16) & 0xff;
  buffer[3] = (value >> 24) & 0xff;
}

#endif
