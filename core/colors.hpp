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


#warning we should ensure input domain for r, g and b is what we want it to be
// rrrgggbb
inline uint8_t color8(uint8_t r, uint8_t g, uint8_t b)
{
  return (r & 0xE0) | ((g >> 3) & 0x1C) | ((b >> 6) & 0x03);
}

// 0rrrrrgggggbbbbb
inline uint16_t color15(const uint8_t r, const uint8_t g, const uint8_t b)
{
  return ((r << 7) & 0x7C00)|((g << 2) & 0x03D0) | (b >> 5);
}

inline void splitcolor15(uint8_t & r, uint8_t & g, uint8_t & b, uint32_t c)
{
  r = ((c >> 7) & 0xf8) | ((c >> 12) & 0x7);
  g = ((c >> 2) & 0xf8) | ((c >>  8) & 0x7);
  b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7);
}

// rrrrrrggggggbbbbb
//
inline uint16_t color16(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r << 8) & 0xFC00)|((g << 2) & 0x03F0) | (b << 3);
}

inline void splitcolor16(uint8_t & r, uint8_t & g, uint8_t & b, uint32_t c)
{
  r = ((c >> 8) & 0xf8) | ((c >> 13) & 0x7);
  g = ((c >> 3) & 0xfc) | ((c >>  9) & 0x3);
  b = ((c << 3) & 0xf8) | ((c >>  2) & 0x7);
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
  r = (c >> 16) & 0xff;
  g = (c >> 8) & 0xff;
  b = c & 0xff;
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
        return color24BGR(red, green, blue);
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
    // RGBcolor palette[256];

    RGBcolor mkcolor(uint8_t red, uint8_t green, uint8_t blue){
        uint32_t my_color = 0;
        switch (this->bpp){
            case 8:
                my_color = color8(red, green, blue);
            break;
            case 15:
                my_color = color15(red, green, blue);
            break;
            case 16:
                my_color = color16(red, green, blue);
            break;
            default:
            case 24:
                my_color = color24BGR(red, green, blue);
            break;
        }
        return my_color;
    }

    void get_palette(RGBPalette & palette) const {
        //assert(bpp <= 8);
        if (bpp > 8) {
            return;
        }
        /* rgb332 */
        for (int bindex = 0; bindex < 4; bindex++) {
            for (int gindex = 0; gindex < 8; gindex++) {
                for (int rindex = 0; rindex < 8; rindex++) {
                    palette[(bindex << 6) | (gindex << 3) | rindex] =
                        (RGBcolor)(
                        (((rindex<<5)|(rindex<<2)|(rindex>>1))<<16) |
                        (((gindex<<5)|(gindex<<2)|(gindex>>1))<< 8) |
                         ((bindex<<6)|(bindex<<4)|(bindex<<2)|(bindex)));
                }
            }
        }
    }


    Colors(int bpp)
    {
        this->bpp = bpp;
        this->black      = mkcolor(0, 0, 0);
        this->grey       = mkcolor(0xc0, 0xc0, 0xc0);
        this->dark_grey  = mkcolor(0x80, 0x80, 0x80);
        this->anthracite = mkcolor(0x80, 0x80, 0x80);
        this->blue       = mkcolor(0x00, 0x00, 0xff);
        this->dark_blue  = mkcolor(0x00, 0x00, 0x7f);
        this->white      = mkcolor(0xff, 0xff, 0xff);
        this->red        = mkcolor(0xff, 0x00, 0x00);
        this->pink       = mkcolor(0xff, 0x00, 0xff);
        this->green      = mkcolor(0x00, 0xff, 0x00);
        this->yellow     = mkcolor(0x00, 0xff, 0xff);

        this->wabgreen   = mkcolor(0x91, 0xbe, 0x3b);
        #warning : colors are to be changed later on
        this->blue       = this->wabgreen;
        this->dark_blue  = this->wabgreen;
    }

    // RGBColor black() { return mkcolor(0, 0, 0);}

};

#endif
