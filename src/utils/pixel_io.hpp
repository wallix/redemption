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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once


static inline uint8_t get_pixel_1bpp(const uint8_t* data, size_t line_bytes, size_t x, size_t y)
{
    const size_t index =  y * line_bytes * 8 + x;

    return ((data[index / 8] & (1 << (7 - (index % 8)))) ? 0xFF : 0);
}

static inline void put_pixel_1bpp(uint8_t* data, size_t line_bytes, size_t x, size_t y, uint8_t value) {
    const size_t index =  y * line_bytes * 8 + x;

    if (value) {
        data[index / 8] |= (1 << (7 - (index % 8)));
    }
    else {
        data[index / 8] &= ~(1 << (7 - (index % 8)));
    }
}

static inline uint32_t get_pixel_24bpp(const uint8_t* data, size_t line_bytes, size_t x, size_t y)
{
    const uint8_t* dest = data + y * line_bytes + x * 3;

    uint32_t res = 0;
    res |= dest[0] << (8 * 0);
    res |= dest[1] << (8 * 1);
    res |= dest[2] << (8 * 2);

    return res;
}

static inline void put_pixel_24bpp(uint8_t* data, size_t line_bytes, size_t x, size_t y, uint32_t value) {
    uint8_t* dest = data + y * line_bytes + x * 3;

    dest[0] = static_cast<uint8_t>(value >> (8 * 0));
    dest[1] = static_cast<uint8_t>(value >> (8 * 1));
    dest[2] = static_cast<uint8_t>(value >> (8 * 2));
}

static inline void put_pixel_32bpp(uint8_t* data, size_t line_bytes, size_t x, size_t y, uint32_t value) {
    uint8_t* dest = data + (y * line_bytes) + (x * 4);

    dest[0] = static_cast<uint8_t>(value >> (8 * 0));
    dest[1] = static_cast<uint8_t>(value >> (8 * 1));
    dest[2] = static_cast<uint8_t>(value >> (8 * 2));
    dest[3] = static_cast<uint8_t>(value >> (8 * 3));
}

static inline void put_pixel_24alpha(uint8_t* data, size_t line_bytes, size_t x, size_t y, uint32_t value) {
    uint8_t* dest = data + y * line_bytes + x * 3;
    uint8_t frontAlpha = (value >> (8 * 3)) & 0xff;

    if (frontAlpha == 0xff) {
        /* front is totally opaque */
        dest[0] = static_cast<uint8_t>(value >> (8 * 0));
        dest[1] = static_cast<uint8_t>(value >> (8 * 1));
        dest[2] = static_cast<uint8_t>(value >> (8 * 2));
    } else if (frontAlpha != 0x00) {
        /* front is not totally transparent */
        uint8_t backAlpha = (0xff - frontAlpha);

        dest[0] = static_cast<uint8_t>( ((dest[0] * backAlpha) + frontAlpha * (value & 0xff)) / 0xff);
        dest[1] = static_cast<uint8_t>( ((dest[1] * backAlpha) + frontAlpha * ((value >> 8) & 0xff)) / 0xff);
        dest[2] = static_cast<uint8_t>( ((dest[2] * backAlpha) + frontAlpha * ((value >> 16) & 0xff)) / 0xff);
    }
}
