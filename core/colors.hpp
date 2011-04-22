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
inline uint16_t color16(uint8_t r, uint8_t g, uint8_t b)
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

inline uint32_t color24RGB(uint8_t r, uint8_t g, uint8_t b)
{
  return (r << 16) | (g << 8) | b;
}

inline uint32_t color24BGR(uint8_t r, uint8_t g, uint8_t b)
{
  return color24RGB(b, g, r);
}

#warning we should have **two** splitcolor32, RGB and BGR
inline void splitcolor32(uint8_t & r, uint8_t & g, uint8_t & b, uint32_t c)
{
  r = c >> 16;
  g = c >> 8;
  b = c;
}

#warning way too complicated, fix that
inline uint32_t color_convert(const uint32_t in_pixel, const uint8_t in_bpp, const uint8_t out_bpp, const uint32_t (& palette)[256])
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    switch (in_bpp){
    case 8:
        splitcolor32(red, green, blue, palette[in_pixel]);
    break;
    case 15:
        splitcolor15(red, green, blue, in_pixel);
    break;
    case 16:
        splitcolor16(red, green, blue, in_pixel);
    break;
    case 32:
    case 24:
        splitcolor32(red, green, blue, in_pixel);
    break;
    default:
        assert(false);
    break;
    }

    switch (out_bpp){
    case 8:
        return color8(red, green, blue);
    break;
    case 15:
        return color15(red, green, blue);
    break;
    case 16:
        return color16(red, green, blue);
    break;
    case 32:
    case 24:
        return color24RGB(red, green, blue);
    break;
    default:
        assert(false);
    break;
    }

    return 0;
}

/* generic colors */
struct Colors {
    int bpp;
    RGBcolor black, grey, dark_grey, blue, dark_blue,
             white, red, green, pink, yellow, anthracite,
             wabgreen;

    void get_palette(RGBPalette & palette) const {
        //assert(bpp <= 8);
        if (bpp > 8) {
            return;
        }
        /* rgb332 */
        for (int bindex = 0; bindex < 4; bindex++) {
            for (int gindex = 0; gindex < 8; gindex++) {
                for (int rindex = 0; rindex < 8; rindex++) {
                    palette[(rindex << 5) | (gindex << 2) | bindex] =
                    (RGBcolor)(
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


    Colors(int bpp)
    {
        this->bpp = bpp;
        uint32_t palette[256];
        if (bpp <= 8){
            this->get_palette(palette);
        }
        this->black      = color_convert(0x000000, 24, bpp, palette);
        this->grey       = color_convert(0xc0c0c0, 24, bpp, palette);
        this->dark_grey  = color_convert(0x808080, 24, bpp, palette);
        this->anthracite = color_convert(0x808080, 24, bpp, palette);
        this->blue       = color_convert(0x0000ff, 24, bpp, palette);
        this->dark_blue  = color_convert(0x00007f, 24, bpp, palette);
        this->white      = color_convert(0xffffff, 24, bpp, palette);
        this->red        = color_convert(0xff0000, 24, bpp, palette);
        this->pink       = color_convert(0xff00ff, 24, bpp, palette);
        this->green      = color_convert(0x00ff00, 24, bpp, palette);
        this->yellow     = color_convert(0x00ffff, 24, bpp, palette);
        this->wabgreen   = color_convert(0x3bbe91, 24, bpp, palette);
        #warning : colors are to be changed later on
        this->blue       = this->wabgreen;
        this->dark_blue  = this->wabgreen;
    }
};

#endif
