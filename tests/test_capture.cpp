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
#include "../core/ssl_calls.hpp"
#include "../utils/png.hpp"
#include "../capture/staticcapture.hpp"
#include "../capture/drawable.hpp"

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

void test_scrblt(const uint8_t rop, const int cx, const int cy, const char * name, const char * shasig){
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    uint8_t bpp = 24;
    Rect screen_rect(0, 0, width, height);
    BGRPalette palette;
    init_palette332(palette);
    Drawable gd(width, height, bpp, palette, false);
    gd.draw(RDPOpaqueRect(Rect(90, 90, 120, 120), RED), screen_rect);
    gd.draw(RDPOpaqueRect(screen_rect, BLUE), Rect(100, 100, 100, 100));
    gd.draw(RDPOpaqueRect(Rect(120, 120, 60, 60), PINK), Rect(100, 100, 100, 100));
    gd.scrblt(90, 90, Rect(300, 300, 120, 120), 0xCC);
    gd.scrblt(90, 90, Rect(90 + cx, 90 + cy, 120, 120), rop);
    SSL_SHA1 sha1;
    uint8_t sig[20];
    ssllib ssl;
    ssl.sha1_init(&sha1);
    for (size_t y = 0; y < (size_t)gd.full.cy; y++){
        ssl.sha1_update(&sha1, gd.data + y * gd.rowsize, gd.rowsize);
    }
    ssl.sha1_final(&sha1, sig);

    if (memcmp(shasig, sig, 20)){
        char buffer[1024];
        sprintf(buffer, "Expected signature: \""
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
        sig[ 0], sig[ 1], sig[ 2], sig[ 3],
        sig[ 4], sig[ 5], sig[ 6], sig[ 7],
        sig[ 8], sig[ 9], sig[10], sig[11],
        sig[12], sig[13], sig[14], sig[15],
        sig[16], sig[17], sig[18], sig[19]);
        BOOST_CHECK_MESSAGE(false, buffer);
    }
    // here compute sha1 of drawable and check for expected value
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_scrblt_%s_XXXXXX.png", name);
    int fd = ::mkostemps(tmpname, 4, O_WRONLY|O_CREAT);
    FILE * f = fdopen(fd, "wb");
    ::dump_png24(f, gd.data, gd.full.cx, gd.full.cy, gd.rowsize);
    ::fflush(f);
    ::fclose(f);
    ::unlink(tmpname);
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltDown)
{
    test_scrblt(0x00, 0, 20, "down00",
    "\x3c\x39\xae\x2b\x84\x5b\xc6\xa8\x75\xc1\xaf\xbb\x5c\x26\xa9\x1f\x94\x24\xc4\x68");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltRight)
{
    test_scrblt(0x00, 20, 0, "right00",
    "\xcc\xe9\x1b\x97\x5e\x2e\x87\xdd\x56\xb6\x9d\x0e\x47\xce\xda\x1e\x7c\x19\x35\x4b");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeft)
{
    test_scrblt(0x00, -20, 0, "left00",
    "\x8d\xd3\xf4\x66\x35\x58\x7b\xee\x15\x0b\xad\xc0\x43\x8a\xe4\x07\x71\xaa\xc7\x0d");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltUp)
{
    test_scrblt(0x00, 0, -20, "up00",
    "\xe8\x47\xa1\x86\x2f\x61\xe5\xb6\x4e\xcb\xf4\xe4\x3c\x4c\xa7\x43\xe9\x20\xc3\x2f");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeftUp)
{
    test_scrblt(0x00, -20, -20, "left_up00",
    "\x7b\xec\x31\x5a\x53\x42\x0d\xb3\x48\xc0\xf2\xdc\x85\x38\x3c\x85\x48\xc4\xc2\xe7");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltDown11)
{
    test_scrblt(0x11, 0, 20, "down11",
    "\x21\x29\x04\x7f\xf7\x49\xfc\x70\xcf\xec\x3a\xfc\x7e\x99\x44\x2e\x88\xed\x99\x32");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltRight11)
{
    test_scrblt(0x11, 20, 0, "right11",
    "\x7e\x74\xa5\x21\x20\x49\xbd\x88\x20\x5b\xeb\x6b\xf7\x08\x46\xec\x68\x6b\x76\x29");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeft11)
{
    test_scrblt(0x11, -20, 0, "left11",
    "\xc3\x8c\x3b\x77\xf3\xa8\xb6\x37\x54\xa7\xf6\x31\x8e\x74\x1a\x45\x2c\xc9\xc5\xbe");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltUp11)
{
    test_scrblt(0x11, 0, -20, "up11",
    "\xf8\xba\x16\xbd\xa0\xee\x1d\xc5\xcd\x6d\x48\x14\xe5\x5c\xb7\x1b\xdb\xfe\x26\xf6");
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeftUp11)
{
    test_scrblt(0x11, -20, -20, "left_up11",
    "\x7e\xb4\x25\xbd\xca\xf1\x82\xdb\x7e\xd4\x1f\x15\xc0\xe9\xd3\xe7\xa9\xe9\x93\x0c");
}
