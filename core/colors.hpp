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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Colors object. Contains generic colors
*/

#if !defined(__COLORS__)
#define __COLORS__

#include <stdint.h>
#include <assert.h>
#include "log.hpp"

typedef uint32_t BGRColor;
typedef BGRColor BGRPalette[256];

// colorN (variable): an index into the current palette or an RGB triplet
//                    value; the actual interpretation depends on the color
//                    depth of the bitmap data.
// +-------------+------------+------------------------------------------------+
// | Color depth | Field size |                Meaning                         |
// +-------------+------------+------------------------------------------------+
// |       8 bpp |     1 byte |     Index into the current color palette.      |
// +-------------+------------+------------------------------------------------+
// |      15 bpp |    2 bytes | RGB color triplet expressed in 5-5-5 format    |
// |             |            | (5 bits for red, 5 bits for green, and 5 bits  |
// |             |            | for blue).                                     |
// +-------------+------------+------------------------------------------------+
// |      16 bpp |    2 bytes | RGB color triplet expressed in 5-6-5 format    |
// |             |            | (5 bits for red, 6 bits for green, and 5 bits  |
// |             |            | for blue).                                     |
// +-------------+------------+------------------------------------------------+
// |    24 bpp   |    3 bytes |     RGB color triplet (1 byte per component).  |
// +-------------+------------+------------------------------------------------+

static inline BGRColor color_decode(const BGRColor c, const uint8_t in_bpp, const uint32_t (& palette)[256]){
    switch (in_bpp){
    case 8:
      return palette[(uint8_t)c] & 0xFFFFFF;
    case 15:
    {
        // r1 r2 r3 r4 r5 g1 g2 g3 g4 g5 b1 b2 b3 b4 b5
        uint8_t r = ((c >> 7) & 0xf8) | ((c >> 12) & 0x7); // r1 r2 r3 r4 r5 r1 r2 r3
        uint8_t g = ((c >> 2) & 0xf8) | ((c >>  7) & 0x7); // g1 g2 g3 g4 g5 g1 g2 g3
        uint8_t b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
        return (r << 16) | (g << 8) | b;
    }
    break;
    case 16:
    {
        // r1 r2 r3 r4 r5 g1 g2 g3 g4 g5 g6 b1 b2 b3 b4 b5
        uint8_t r = ((c >> 8) & 0xf8) | ((c >> 13) & 0x7); // r1 r2 r3 r4 r5 r6 r7 r8
        uint8_t g = ((c >> 3) & 0xfc) | ((c >>  9) & 0x3); // g1 g2 g3 g4 g5 g6 g1 g2
        uint8_t b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
        return (r << 16) | (g << 8) | b;
    }
    case 32:
    case 24:
      return c & 0xFFFFFF;
    default:
        LOG(LOG_ERR, "in_bpp = %d", in_bpp);
        assert(false);
        break;
    }
    return 0;
}

static inline BGRColor color_decode_opaquerect(const BGRColor c, const uint8_t in_bpp, const uint32_t (& palette)[256]){
    switch (in_bpp){
    case 8:
      return palette[(uint8_t)c] & 0xFFFFFF;
    case 15:
    {
        //  b1 b2 b3 b4 b5 g1 g2 g3 g4 g5r1 r2 r3 r4 r5
        uint8_t b = ((c >> 7) & 0xf8) | ((c >> 12) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
        uint8_t g = ((c >> 2) & 0xf8) | ((c >>  7) & 0x7); // g1 g2 g3 g4 g5 g1 g2 g3
        uint8_t r = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); // r1 r2 r3 r4 r5 r1 r2 r3
        return (r << 16) | (g << 8) | b;
    }
    break;
    case 16:
    {
        //  b1 b2 b3 b4 b5 g1 g2 g3 g4 g5 g6 r1 r2 r3 r4 r5
        uint8_t b = ((c >> 8) & 0xf8) | ((c >> 13) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
        uint8_t g = ((c >> 3) & 0xfc) | ((c >>  9) & 0x3); // g1 g2 g3 g4 g5 g6 g1 g2
        uint8_t r = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); // r1 r2 r3 r4 r5 r6 r7 r8
        return (r << 16) | (g << 8) | b;
    }
    case 32:
    case 24:
      return c & 0xFFFFFF;
    default:
        LOG(LOG_ERR, "in_bpp = %d", in_bpp);
        assert(false);
        break;
    }
    return 0;
}


static inline void init_palette332(BGRPalette & palette)
{
    /* rgb332 palette */
    for (int bindex = 0; bindex < 4; bindex++) {
        for (int gindex = 0; gindex < 8; gindex++) {
            for (int rindex = 0; rindex < 8; rindex++) {
                palette[(rindex << 5) | (gindex << 2) | bindex] =
                (BGRColor)(
                // r1 r2 r2 r1 r2 r3 r1 r2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                    (((rindex<<5)|(rindex<<2)|(rindex>>1))<<16)
                // 0 0 0 0 0 0 0 0 g1 g2 g3 g1 g2 g3 g1 g2 0 0 0 0 0 0 0 0
                   | (((gindex<<5)|(gindex<<2)|(gindex>>1))<< 8)
                // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 b1 b2 b1 b2 b1 b2 b1 b2
                   | ((bindex<<6)|(bindex<<4)|(bindex<<2)|(bindex)));
            }
        }
    }
}

static inline BGRColor RGBtoBGR(const BGRColor & c){
    return ((c << 16) & 0xFF0000)|(c & 0x00FF00)|((c>>16) & 0x0000FF);
}

static inline BGRColor color_encode(const BGRColor c, const uint8_t out_bpp){
    switch (out_bpp){
    case 8:
    // rrrgggbb
        return
        (((c >> 16) & 0xFF)       & 0xE0)
       |((((c >> 8) & 0xFF) >> 3) & 0x1C)
       |(((c        & 0xFF) >> 6) & 0x03);
    break;
    case 15:
    // --> 0 b1 b2 b3 b4 b5 g1 g2 g3 g4 g5 r1 r2 r3 r4 r5
        return
        // b1 b2 b3 b4 b5 b6 b7 b8 --> 0 b1 b2 b3 b4 b5 0 0 0 0 0 0 0 0 0 0
        (((c         & 0xFF) << 7) & 0x7C00)
        // g1 g2 g3 g4 g5 g6 g7 g8 --> 0 0 0 0 0 0 g1 g2 g3 g4 g5 0 0 0 0 0
       |((((c >>  8) & 0xFF) << 2) & 0x03E0)
        // r1 r2 r3 r4 r5 r6 r7 r8 --> 0 0 0 0 0 0 0 0 0 0 0 r1 r2 r3 r4 r5
       | (((c >> 16) & 0xFF) >> 3);
    break;
    case 16:
    // --> b1 b2 b3 b4 b5 g1 g2 g3 g4 g5 g6 r1 r2 r3 r4 r5
        return
        // b1 b2 b3 b4 b5 b6 b7 b8 --> b1 b2 b3 b4 b5 0 0 0 0 0 0 0 0 0 0 0
        (((c         & 0xFF) << 8) & 0xF800)
        // g1 g2 g3 g4 g5 g6 g7 g8 --> 0 0 0 0 0 g1 g2 g3 g4 g5 g6 0 0 0 0 0
       |((((c >>  8) & 0xFF) << 3) & 0x07E0)
        // r1 r2 r3 r4 r5 r6 r7 r8 --> 0 0 0 0 0 0 0 0 0 0 0 r1 r2 r3 r4 r5
       | (((c >> 16) & 0xFF) >> 3);
    case 32:
    case 24:
        return c;
    default:
        assert(false);
    break;
    }
    return 0;
}

enum {
    BLACK      = 0x000000,
    GREY       = 0xc0c0c0,
    DARK_GREY  = 0x808080,
    ANTHRACITE = 0x808080,
    BLUE       = 0xff0000,
    DARK_BLUE  = 0x7f0000,
    WHITE      = 0xffffff,
    RED        = 0x0000ff,
    PINK       = 0xff00ff,
    GREEN      = 0x00ff00,
    YELLOW     = 0xffff00,
    WABGREEN   = 0x2BBE91,
    DARK_WABGREEN = 0x2BBE91,
    WINBLUE = 0x9C4D00,
};

#endif
