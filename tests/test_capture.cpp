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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Martin Potier

   Unit test to write / read a "movie" from a file
   Using lib boost functions for testing
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapture
#include <errno.h>
#include <boost/test/auto_unit_test.hpp>
#include <algorithm>

#include "../utils/png.hpp"

#include <staticcapture.hpp>
#include <drawable.hpp>

BOOST_AUTO_TEST_CASE(TestCreateCapture)
{
    // Create a simple capture image and dump it to file
    Rect screen_rect(0, 0, 640, 480);
    StaticCapture gd(screen_rect.cx, screen_rect.cy, 24, NULL, NULL, NULL);
    gd.opaque_rect(RDPOpaqueRect(screen_rect, WHITE), screen_rect);
    gd.opaque_rect(RDPOpaqueRect(screen_rect.shrink(5), BLACK), screen_rect);
    uint16_t y = screen_rect.cy - 1;
    for (uint16_t x = 0 ; x < screen_rect.cx ; x++){
        gd.line_to(RDPLineTo(0, 0, 0, x, y, BLUE, 0xCC, RDPPen(0, 1, GREEN)), screen_rect);
        gd.line_to(RDPLineTo(0, x, y, 0, 0, WHITE, 0xCC, RDPPen(0, 1, BLACK)), screen_rect);
    }

    y = screen_rect.cy - 1;
    for (uint16_t x = 0 ; x < screen_rect.cx ; x++){
        gd.line_to(RDPLineTo(0, screen_rect.cx - 1, 0, x, y, BLUE, 0xCC, RDPPen(0, 1, RED)), screen_rect);
        gd.line_to(RDPLineTo(0, x, y, screen_rect.cx - 1, 0, WHITE, 0xCC, RDPPen(0, 1, BLACK)), screen_rect);
    }
    gd.dump_png();
}

BOOST_AUTO_TEST_CASE(TestDrawable)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    uint8_t bpp = 24;
    Rect screen_rect(0, 0, width, height);
    BGRPalette palette;
    init_palette332(palette);
    Drawable gd(width, height, bpp, palette, false);
    gd.draw(RDPOpaqueRect(screen_rect, RED), screen_rect);
    gd.draw(RDPOpaqueRect(screen_rect, BLUE), Rect(100, 100, 100, 100));
    gd.draw(RDPOpaqueRect(Rect(120, 120, 60, 60), PINK), Rect(100, 100, 100, 100));
    gd.scrblt(90, 90, Rect(300, 300, 120, 120));
    gd.scrblt(90, 90, Rect(90, 110, 120, 120));
 // this one should not change anything, hence we should test CRC on image
//    gd.scrblt(300, 300, Rect(90, 110, 120, 120));

//    gd.scrblt(90, 90, Rect(110, 90, 120, 120));
//    gd.scrblt(90, 90, Rect(70, 90, 120, 120));
//    gd.scrblt(90, 90, Rect(90, 70, 120, 120));
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_png_XXXXXX.png");
    int fd = ::mkostemps(tmpname, 4, O_WRONLY|O_CREAT);
    FILE * f = fdopen(fd, "wb");
    ::dump_png24(f, gd.data, gd.screen.cx, gd.screen.cy, gd.rowsize);
    ::fflush(f);
    ::fclose(f);
//    ::unlink(tmpname);

}
