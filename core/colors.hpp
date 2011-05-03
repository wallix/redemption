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

typedef uint32_t RGBcolor;
typedef RGBcolor RGBPalette[256];

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

#warning we should ensure input domain for r, g and b is what we want it to be
// rrrgggbb
inline uint8_t color8(uint8_t r, uint8_t g, uint8_t b)
{
  return (r & 0xE0) | ((g >> 3) & 0x1C) | ((b >> 6) & 0x03);
}

// 0 r1 r2 r3 r4 r5 g1 g2 g3 g4 g5 b1 b2 b3 b4 b5
inline uint16_t color15(const uint8_t r, const uint8_t g, const uint8_t b)
{
   // r1 r2 r3 r4 r5 r6 r7 r8
  return ((r << 7) & 0x7C00) // 0 r1 r2 r3 r4 r5 0 0 0 0 0 0 0 0 0 0
    // g1 g2 g3 g4 g5 g6 g7 g8
    |((g << 2) & 0x03E0) // 0 0 0 0 0 0 g1 g2 g3 g4 g5 0 0 0 0 0
    // b1 b2 b3 b4 b5 b6 b7 b8
    | (b >> 3); // 0 0 0 0 0 0 0 0 0 0 0 b1 b2 b3 b4 b5
}

inline void splitcolor15(uint8_t & r, uint8_t & g, uint8_t & b, uint32_t c)
{
  // r1 r2 r3 r4 r5 g1 g2 g3 g4 g5 b1 b2 b3 b4 b5
  r = ((c >> 7) & 0xf8) | ((c >> 12) & 0x7); // r1 r2 r3 r4 r5 r1 r2 r3
  g = ((c >> 2) & 0xf8) | ((c >>  7) & 0x7); // g1 g2 g3 g4 g5 g1 g2 g3
  b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
}

// r1 r2 r3 r4 r5 g1 g2 g3 g4 g5 g6 b1 b2 b3 b4 b5
inline uint32_t color16(uint8_t r, uint8_t g, uint8_t b)
{
   // r1 r2 r3 r4 r5 r6 r7 r8
   return ((r << 8) & 0xF800) // r1 r2 r3 r4 r5 0 0 0 0 0 0 0 0 0 0 0
       // g1 g2 g3 g4 g5 g6 g7 g8
       |((g << 3) & 0x07E0)       // 0 0 0 0 0 g1 g2 g3 g4 g5 g6 0 0 0 0 0
       // b1 b2 b3 b4 b5 b6 b7 b8
       | (b >> 3);                // 0 0 0 0 0 0 0 0 0 0 0 b1 b2 b3 b4 b5
}

inline void splitcolor16(uint8_t & r, uint8_t & g, uint8_t & b, uint32_t c)
{
  // r1 r2 r3 r4 r5 g1 g2 g3 g4 g5 g6 b1 b2 b3 b4 b5
  r = ((c >> 8) & 0xf8) | ((c >> 13) & 0x7); // r1 r2 r3 r4 r5 r6 r7 r8
  g = ((c >> 3) & 0xfc) | ((c >>  9) & 0x3); // g1 g2 g3 g4 g5 g6 g1 g2
  b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
}

inline uint32_t color24RGB(const uint8_t r, const uint8_t g, const uint8_t b)
{
  return (r << 16) | (g << 8) | b;
}

inline uint32_t color24BGR(const uint8_t r, const uint8_t g, const uint8_t b)
{
  return color24RGB(b, g, r);
}

inline void splitcolor32RGB(uint8_t & r, uint8_t & g, uint8_t & b, uint32_t c)
{
  r = c >> 16;
  g = c >> 8;
  b = c;
}

inline void splitcolor32BGR(uint8_t & r, uint8_t & g, uint8_t & b, uint32_t c)
{
  b = c >> 16;
  g = c >> 8;
  r = c;
}

inline uint32_t color_decode(const uint32_t in_pixel, const uint8_t in_bpp, const uint32_t (& palette)[256]){
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    switch (in_bpp){
    case 8:
        splitcolor32RGB(red, green, blue, palette[in_pixel]);
    break;
    case 15:
        splitcolor15(red, green, blue, in_pixel);
    break;
    case 16:
        splitcolor16(red, green, blue, in_pixel);
    break;
    case 32:
    case 24:
        splitcolor32RGB(red, green, blue, in_pixel);
    break;
    default:
        LOG(LOG_ERR, "in_bpp = %d", in_bpp);
        assert(false);
    break;
    }

    return (red << 16) | (green << 8) | blue;
}


inline uint32_t color_encodeBGR24(const uint32_t pix, const uint8_t out_bpp, const uint32_t (& palette)[256]){
    uint32_t res = 0;
    switch (out_bpp){
    case 8:
        res = color8(pix & 0xFF, (pix >> 8) & 0xFF, (pix >> 16) & 0xFF);
    break;
    case 15:
        res = color15(pix & 0xFF, (pix >> 8) & 0xFF, (pix >> 16) & 0xFF);
    break;
    case 16:
        res = color16(pix & 0xFF, (pix >> 8) & 0xFF, (pix >> 16) & 0xFF);
    break;
    case 32:
    case 24:
        res = color24RGB(pix & 0xFF, (pix >> 8) & 0xFF, (pix >> 16) & 0xFF);
    break;
    default:
        assert(false);
    break;
    }

    return res;
}


inline uint32_t color_encode(const uint32_t in_pixel, const uint8_t out_bpp, const uint32_t (& palette)[256]){
    uint32_t res = 0;
    switch (out_bpp){
    case 8:
        res = color8((in_pixel >> 16) & 0xFF, (in_pixel >> 8) & 0xFF, in_pixel & 0xFF);
    break;
    case 15:
        res = color15((in_pixel >> 16) & 0xFF, (in_pixel >> 8) & 0xFF, in_pixel & 0xFF);
    break;
    case 16:
        res = color16((in_pixel >> 16) & 0xFF, (in_pixel >> 8) & 0xFF, in_pixel & 0xFF);
    break;
    case 32:
    case 24:
        res = color24RGB((in_pixel >> 16) & 0xFF, (in_pixel >> 8) & 0xFF, in_pixel & 0xFF);
    break;
    default:
        assert(false);
    break;
    }

    return res;
}

inline uint32_t color_convert(const uint32_t in_pixel, const uint8_t in_bpp, const uint8_t out_bpp, const uint32_t (& palette)[256])
{
    return color_encode(color_decode(in_pixel, in_bpp, palette), out_bpp, palette);
}

enum {
    BLACK      = 0x000000,
    GREY       = 0xc0c0c0,
    DARK_GREY  = 0x808080,
    ANTHRACITE = 0x808080,
    BLUE       = 0x0000ff,
    DARK_BLUE  = 0x00007f,
    WHITE      = 0xffffff,
    RED        = 0xff0000,
    PINK       = 0xff00ff,
    GREEN      = 0x00ff00,
    YELLOW     = 0x00ffff,
    WABGREEN   = 0x91BE2B,
    DARK_WABGREEN = 0x91BE2B,
};

#endif
