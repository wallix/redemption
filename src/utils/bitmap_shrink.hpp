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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <cstdint>

// TODO: see parameters used rowsize if annoying as it's an implicit way
// to provide Bpp (Bytes per pixel) and the function hard encode it to 3.
// There are also annoying rounding behaviors. Fix that.
static inline void scale_data(uint8_t *dest, const uint8_t *src,
                       unsigned int dest_width, unsigned int src_width,
                       unsigned int dest_height, unsigned int src_height,
                       unsigned int src_rowsize) {
    const uint32_t Bpp = 3;
    unsigned int y_pixels = dest_height;
    unsigned int y_int_part = (src_height / dest_height) * src_rowsize;
    unsigned int y_fract_part = src_height % dest_height;
    unsigned int yE = 0;
    unsigned int x_int_part = src_width / dest_width * Bpp;
    unsigned int x_fract_part = src_width % dest_width;

    while (y_pixels-- > 0) {
        unsigned int xE = 0;
        const uint8_t * x_src = src;
        unsigned int x_pixels = dest_width;
        while (x_pixels-- > 0) {
            dest[0] = x_src[2];
            dest[1] = x_src[1];
            dest[2] = x_src[0];

            dest += Bpp;
            x_src += x_int_part;
            xE += x_fract_part;
            if (xE >= dest_width) {
                xE -= dest_width;
                x_src += Bpp;
            }
        }
        src += y_int_part;
        yE += y_fract_part;
        if (yE >= dest_height) {
            yE -= dest_height;
            src += src_rowsize;
        }
    }
}

