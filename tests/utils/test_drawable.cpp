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

   Unit test of Drawing primitive used to create snapshot/movies
   Using lib boost functions for testing
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapture
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include <errno.h>
#include <algorithm>
#include "ssl_calls.hpp"
#include "png.hpp"
#include "RDP/RDPDrawable.hpp"
#include "staticcapture.hpp"
#include "drawable.hpp"

inline bool check_sig(const uint8_t* data, std::size_t height, uint32_t len,
                      char * message, const char * shasig)
{
    uint8_t sig[20];
    SslSha1 sha1;
    for (size_t y = 0; y < (size_t)height; y++){
        sha1.update(StaticStream(data + y * len, len));
    }
    sha1.final(sig);

    if (memcmp(shasig, sig, 20)){
        sprintf(message, "Expected signature: \""
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
        return false;
    }
    return true;
}

inline bool check_sig(Drawable & data, char * message, const char * shasig)
{
    return check_sig(data.data, data.height, data.rowsize, message, shasig);
}

// to see last result file, remove unlink
// and do something like:
// eog `ls -1tr /tmp/test_* | tail -n 1`
// (or any other variation you like)

void dump_png(const char * prefix, const Drawable & data)
{
    char tmpname[128];
    sprintf(tmpname, "%sXXXXXX.png", prefix);
    int fd = ::mkostemps(tmpname, 4, O_WRONLY|O_CREAT);
    FILE * f = fdopen(fd, "wb");
    ::dump_png24(f, data.data, data.width, data.height, data.rowsize);
    ::fclose(f);
}

BOOST_AUTO_TEST_CASE(TestLineTo)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height, true);
    gd.draw(RDPOpaqueRect(screen_rect, WHITE), screen_rect);
    gd.draw(RDPOpaqueRect(screen_rect.shrink(5), BLACK), screen_rect);

    uint16_t y = screen_rect.cy - 1;
    for (uint16_t x = 0 ; x < screen_rect.cx ; x += 40){
        gd.draw(RDPLineTo(0, 0, 0, x, y, BLUE, 0xCC, RDPPen(0, 1, GREEN)), screen_rect);
        gd.draw(RDPLineTo(0, x + 10, y, 0, 0, BLUE, 0xCC, RDPPen(0, 1, RED)), screen_rect);
        gd.draw(RDPLineTo(0, screen_rect.cx - 1, 0, screen_rect.cx - 1 - x, y, BLUE, 0xCC, RDPPen(0, 1, WHITE)), screen_rect);
        gd.draw(RDPLineTo(0, screen_rect.cx - 1 - x + 10, y, screen_rect.cx - 1, 0, BLUE, 0xCC, RDPPen(0, 1, BLUE)), screen_rect);
    }
    gd.draw(RDPLineTo(0, 0, 0, 640, 480, BLUE, 0xCC, RDPPen(0, 1, GREEN)), screen_rect);

    gd.draw(RDPLineTo(0, 0, 0, 1024, 0, BLUE, 0xCC, RDPPen(0, 1, PINK)), screen_rect);
    gd.draw(RDPLineTo(0, 0, 0, 0, 768, BLUE, 0xCC, RDPPen(0, 1, PINK)), screen_rect);
    gd.draw(RDPLineTo(0, 639, 0, 639, 768, BLUE, 0xCC, RDPPen(0, 1, PINK)), screen_rect);
    gd.draw(RDPLineTo(0, 0, 479, 1024, 479, BLUE, 0xCC, RDPPen(0, 1, PINK)), screen_rect);

    gd.draw(RDPLineTo(10, 0, 10, 1024, 479, BLUE, 0xCC, RDPPen(0, 1, PINK)), screen_rect);

    char message[1024];
    if (!check_sig(gd.drawable, message,
        "\x9c\x08\x94\x45\xa6\xa3\x80\xaa\xb6\x3d"
        "\xbb\xca\xfc\x31\x16\xd1\x31\xa0\xe4\x9c")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
//        dump_png("/tmp/test_line_000_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestPatBlt)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height, true);
    gd.draw(RDPPatBlt(screen_rect, 0xFF, WHITE, WHITE, RDPBrush()), screen_rect);
    gd.draw(RDPPatBlt(screen_rect.shrink(5), 0x00, WHITE, WHITE, RDPBrush()), screen_rect);


    gd.draw(RDPOpaqueRect(screen_rect.shrink(10), RED), screen_rect);
     // RED inverted becomes CYAN
    gd.draw(RDPPatBlt(screen_rect.shrink(15), 0x55, WHITE, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(20), RED), screen_rect);
    // Should be Black
    gd.draw(RDPPatBlt(screen_rect.shrink(25), 0x05, 0xFFFF00, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(30), RED), screen_rect);
    // Should be RED
    gd.draw(RDPPatBlt(screen_rect.shrink(35), 0x0F, 0xFFFF00, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(40), GREEN), screen_rect);
    // Should be BLUE
    gd.draw(RDPPatBlt(screen_rect.shrink(45), 0x50, 0xFFFF00, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(50), GREEN), screen_rect);
    // Should be purple
    gd.draw(RDPPatBlt(screen_rect.shrink(55), 0x5A, WHITE, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(60), GREEN), screen_rect);
    // Should be purple
    gd.draw(RDPPatBlt(screen_rect.shrink(65), 0x5F, 0xFFFF00, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(70), 0xFF00FF), screen_rect);
    // Should be blue
    gd.draw(RDPPatBlt(screen_rect.shrink(75), 0xA0, 0xFFFF00, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(80), GREEN), screen_rect);
    // Should be GREEN
    gd.draw(RDPPatBlt(screen_rect.shrink(85), 0xA5, WHITE, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(90), RED), screen_rect);
    // Should be white
    gd.draw(RDPPatBlt(screen_rect.shrink(95), 0xAF, RED, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(100), GREEN), screen_rect);
    // Should be 0x2F2F2F Dark Grey
    gd.draw(RDPPatBlt(screen_rect.shrink(105), 0xF0, 0x2F2F2F, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(110), 0xFF00FF), screen_rect);
    // Should be yellow
    gd.draw(RDPPatBlt(screen_rect.shrink(115), 0xF5, RED, WHITE, RDPBrush()), screen_rect);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(120), RED), screen_rect);
    // Should be purple
    gd.draw(RDPPatBlt(screen_rect.shrink(125), 0xFA, BLUE, WHITE, RDPBrush()), screen_rect);

    uint8_t shasig[20] = {
        0x8e, 0x6b, 0xe0, 0x91, 0x08, 0xce, 0x54, 0x3b, 0x58, 0x3a,
        0x8f, 0x84, 0x6c, 0x59, 0x1c, 0x3e, 0xae, 0x5c, 0x1c, 0xf5
    };
    char message[1024];
    if (!check_sig(gd.drawable, message, (char*)shasig)){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    //   dump_png("/tmp/test_patblt_000_", gd.drawable);
}




BOOST_AUTO_TEST_CASE(TestDestBlt)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height, true);
//    gd.draw(RDPPatBlt(screen_rect, 0xFF, WHITE, WHITE, RDPBrush()), screen_rect);
    gd.draw(RDPDestBlt(screen_rect, 0xFF), screen_rect); // WHITENESS
    gd.draw(RDPDestBlt(screen_rect.shrink(5), 0x00), screen_rect); // BLACKNESS
    gd.draw(RDPOpaqueRect(screen_rect.shrink(10), RED), screen_rect); // RED
     // RED inverted becomes CYAN
    gd.draw(RDPDestBlt(screen_rect.shrink(15), 0x55), screen_rect);

    char message[1024];
    if (!check_sig(gd.drawable, message,
            "\xca\xee\x18\x2c\x77\x53\x70\x93\xfa\xf3"
            "\x58\xda\xd1\x65\x1a\x17\x4d\x7c\xff\xd7")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    //    dump_png("/tmp/test_destblt_000_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestAddMouse)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height, true);
    gd.draw(RDPOpaqueRect(screen_rect, RED), screen_rect); // RED
    gd.drawable.trace_mouse(100, 100);

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
            "\x36\x27\xca\x81\x58\x35\x86\x4c\x20\x90"
            "\x62\x13\x69\x4a\x91\x79\xb5\x18\x42\x1e")){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    // dump_png("/tmp/test_mouse_000_", gd.drawable);

    gd.drawable.clear_mouse();

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
            "\x59\x99\x2f\x37\x8b\x44\x4b\xad\xf0\x10"
            "\x23\x03\xd8\xde\xea\x81\x41\x3b\x12\x0a")){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    // dump_png("/tmp/test_mouse_001_", gd.drawable);
}


BOOST_AUTO_TEST_CASE(TestTimestampMouse)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height, true);
    gd.draw(RDPOpaqueRect(screen_rect, RED), screen_rect); // RED

    time_t rawtime;
    time(&rawtime);
    struct tm now;

    now.tm_sec  =  51;
    now.tm_min  =  11;
    now.tm_hour =  13;
    now.tm_mday =   8;
    now.tm_mon  =   2;
    now.tm_year = 112;
    now.tm_wday =   4;
    now.tm_yday =  67;
    now.tm_isdst =  0;

    gd.drawable.trace_timestamp(now);

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
            "\x69\xc8\xb0\x41\x33\x12\xf8\x13\xe7\x20"
            "\x6b\xa2\xdb\x2d\x58\x89\xa3\xea\x76\x71")){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    //dump_png("/tmp/test_timestamp_000_", gd.drawable);


    now.tm_sec  =  00;
    now.tm_min  =  12;
    now.tm_hour =  13;
    now.tm_mday =   8;
    now.tm_mon  =   2;
    now.tm_year = 113;
    now.tm_wday =   4;
    now.tm_yday =  67;
    now.tm_isdst =  0;

    gd.drawable.clear_timestamp();
    gd.drawable.trace_timestamp(now);

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
            "\xc3\xd3\xc4\x03\x65\x7e\xe3\xcf\x1a\x94"
            "\xb0\xa7\x18\x14\x66\xa9\xc8\x78\xb4\x63")){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    //dump_png("/tmp/test_timestamp_001_", gd.drawable);


    gd.drawable.clear_timestamp();

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
            "\x59\x99\x2f\x37\x8b\x44\x4b\xad\xf0\x10"
            "\x23\x03\xd8\xde\xea\x81\x41\x3b\x12\x0a")){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    //dump_png("/tmp/test_timestamp_002_", gd.drawable);

}

TODO("We should perform exhaustive tests on scrblt like for patblt, current tests are not exhaustive.")
void test_scrblt(const uint8_t rop, const int cx, const int cy, const char * name, const char * shasig){
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height, true);
    gd.draw(RDPOpaqueRect(Rect(90, 90, 120, 120), RED), screen_rect);
    gd.draw(RDPOpaqueRect(screen_rect, BLUE), Rect(100, 100, 100, 100));
    gd.draw(RDPOpaqueRect(Rect(120, 120, 60, 60), PINK), Rect(100, 100, 100, 100));
    gd.drawable.scrblt(90, 90, Rect(300, 300, 120, 120), 0xCC);
    gd.drawable.scrblt(90, 90, Rect(90 + cx, 90 + cy, 120, 120), rop);

    char message[1024];
    if (!check_sig(gd.drawable, message, shasig)){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    // char tmpname[128];
    // sprintf(tmpname, "/tmp/test_scrblt_%s", name);
    // dump_png(tmpname, gd.drawable);
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


BOOST_AUTO_TEST_CASE(TestMemblt)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    BGRPalette palette;
    init_palette332(palette);

    RDPDrawable gd(width, height, true);
    gd.draw(RDPOpaqueRect(screen_rect, 0x2F2F2F), screen_rect);
    gd.draw(RDPOpaqueRect(Rect(100,100,20, 20), BLUE), screen_rect);

    uint8_t comp64x64RED[] = { 0xc0, 0x30, 0x00, 0x00, 0xFF, 0xf0, 0xc0, 0x0f, };
    BGRPalette palette332;
    init_palette332(palette332);
    Bitmap * bmp = new Bitmap(24, &palette332, 64, 64, comp64x64RED, sizeof(comp64x64RED), true );

    // red square
    gd.draw(RDPMemBlt(1, Rect(5, 5, 20, 20), 0xCC, 0, 0, 10), screen_rect, *bmp);
    // inverted red square (cyan)
    gd.draw(RDPMemBlt(1, Rect(25, 25, 20, 20), 0x55, 0, 0, 10), screen_rect, *bmp);
    // black square
    gd.draw(RDPMemBlt(1, Rect(45, 45, 20, 20), 0x00, 0, 0, 10), screen_rect, *bmp);
    // white square
    gd.draw(RDPMemBlt(1, Rect(65, 65, 20, 20), 0xFF, 0, 0, 10), screen_rect, *bmp);

    char message[1024];
    if (!check_sig(gd.drawable, message,
    "\x6b\x51\x02\x43\xfd\xb5\x37\x97\x8e\x7e"
    "\x80\xf9\xce\x74\xcb\x7e\x34\x7d\xb8\xe6")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    //dump_png("/tmp/test_memblt_", gd.drawable);
}


BOOST_AUTO_TEST_CASE(TestBgr2RGB)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    BGRPalette palette;
    init_palette332(palette);

    RDPDrawable gd(width, height, true);
    gd.draw(RDPOpaqueRect(screen_rect, 0xFF0000), screen_rect);
    gd.drawable.bgr2rgb();

    char message[1024];
    if (!check_sig(gd.drawable, message,
    "\x59\x99\x2f\x37\x8b\x44\x4b\xad\xf0\x10\x23\x03\xd8\xde\xea\x81\x41\x3b\x12\x0a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
//    dump_png("./testBGR2RGB", gd.drawable);

}
