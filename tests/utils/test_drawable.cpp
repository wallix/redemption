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
   Copyright (C) Wallix 2011-2013
   Author(s): Christophe Grosjean, Martin Potier

   Unit test of Drawing primitive used to create snapshot/movies
   Using lib boost functions for testing
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapture
#include <boost/test/auto_unit_test.hpp>

//#define LOGPRINT
#define LOGNULL
#include "log.hpp"

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
    for (size_t y = 0; y < static_cast<size_t>(height); y++){
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
    ::dump_png24(f, data.data, data.width, data.height, data.rowsize, true);
    ::fclose(f);
}
void save_to_png(const char * filename, const Drawable & data)
{
    FILE * file = fopen(filename, "w+");
    dump_png24(file, data.data, data.width,
               data.height, data.rowsize, true);
    fclose(file);
}


BOOST_AUTO_TEST_CASE(TestLineTo)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
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

    gd.draw(RDPLineTo(10, 0, 10, 1024, 479, BLUE, 0xCC, RDPPen(0, 1, PINK)), screen_rect.shrink(5));

    char message[1024];
    if (!check_sig(gd.drawable, message,
                   "\x1c\x41\x1d\xab\xe8\x8c\x5b\x40\xd1\x7b"
                   "\x62\xa2\xb0\x68\x0e\x5c\x36\x55\xae\x24"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    // save_to_png("/tmp/test_line_000.png", gd.drawable);
    // dump_png("/tmp/test_line_005_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestPolyline)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    gd.draw(RDPOpaqueRect(screen_rect, WHITE), screen_rect);
    gd.draw(RDPOpaqueRect(screen_rect.shrink(5), BLACK), screen_rect);

    BStream deltaPoints(1024);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(20);

    deltaPoints.out_sint16_le(160);
    deltaPoints.out_sint16_le(0);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(-30);

    deltaPoints.out_sint16_le(50);
    deltaPoints.out_sint16_le(50);

    deltaPoints.out_sint16_le(-50);
    deltaPoints.out_sint16_le(50);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(-30);

    deltaPoints.out_sint16_le(-160);
    deltaPoints.out_sint16_le(0);

    deltaPoints.mark_end();
    deltaPoints.rewind();

    gd.draw(RDPPolyline(158, 230, 0x06, 0, 0xFFFFFF, 7, deltaPoints), screen_rect);

    char message[1024];
    if (!check_sig(gd.drawable, message,
    "\x32\x60\x8b\x02\xb9\xa2\x83\x27\x0f\xa9\x67\xef\x3c\x2e\xa0\x25\x69\x16\x02\x2b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    // dump_png("/tmp/test_polyline_000_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestEllipse)
{
    uint16_t width = 1280;
    uint16_t height = 1024;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    gd.draw(RDPOpaqueRect(screen_rect, WHITE), screen_rect);
    gd.draw(RDPOpaqueRect(screen_rect.shrink(5), LIGHT_GREEN), screen_rect);

    unsigned long long usec = ustime();
    unsigned long long cycles = rdtsc();

    gd.draw(RDPEllipseSC(Rect(2, 200, 540, 32), BLUE, 0x06, 0x01), screen_rect);

    unsigned long long elapusec = ustime() - usec;
    unsigned long long elapcyc = rdtsc() - cycles;
    LOG(LOG_INFO, "elapsed time = %llu %llu %f\n", elapusec, elapcyc, (double)elapcyc / (double)elapusec);

    usec = ustime();
    cycles = rdtsc();

    gd.draw(RDPEllipseSC(Rect(100, 2, 40, 400), RED, 0x06, 0x01), screen_rect);

    elapusec = ustime() - usec;
    elapcyc = rdtsc() - cycles;
    LOG(LOG_INFO, "elapsed time = %llu %llu %f\n", elapusec, elapcyc, (double)elapcyc / (double)elapusec);

    usec = ustime();
    cycles = rdtsc();

    gd.draw(RDPEllipseSC(Rect(2, 300, 540, 32), BLUE, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(200, 2, 40, 400), RED, 0x06, 0x00), screen_rect);

    gd.draw(RDPEllipseSC(Rect(2, 600, 540, 32), BLUE, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(200, 500, 40, 401), RED, 0x0D, 0x00), screen_rect);

    gd.draw(RDPEllipseSC(Rect(2, 610, 540, 32), BLUE, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(300, 500, 40, 401), RED, 0x0D, 0x01), screen_rect);



    gd.draw(RDPEllipseSC(Rect(700, 12, 6, 6), BLUE, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(715, 12, 6, 6), BLUE, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(730, 12, 6, 6), BLUE, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(745, 12, 6, 6), BLUE, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(700, 28, 5, 5), GREEN, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(715, 28, 5, 5), GREEN, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(730, 28, 5, 5), GREEN, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(745, 28, 5, 5), GREEN, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(800, 12, 8, 8), BLUE, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(815, 12, 8, 8), BLUE, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(830, 12, 8, 8), BLUE, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(845, 12, 8, 8), BLUE, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(800, 38, 15, 15), GREEN, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(815, 38, 15, 15), GREEN, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(830, 38, 15, 15), GREEN, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(845, 38, 15, 15), GREEN, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(800, 88, 5, 15), GREEN, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(815, 88, 5, 15), GREEN, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(830, 88, 5, 15), GREEN, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(845, 88, 5, 15), GREEN, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(700, 88, 10, 10), GREEN, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(715, 88, 10, 10), GREEN, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(730, 88, 10, 10), GREEN, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(745, 88, 10, 10), GREEN, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(700, 888, 40, 30), RED, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(750, 888, 40, 30), GREEN, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(800, 888, 40, 30), MEDIUM_BLUE, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(850, 888, 40, 30), BLUE, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(700, 930, 30, 40), RED, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(750, 930, 30, 40), GREEN, 0x0D, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(800, 930, 30, 40), MEDIUM_BLUE, 0x06, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(850, 930, 30, 40), BLUE, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(700, 600, 230, 140), RED, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(750, 530, 310, 240), GREEN, 0x07, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(800, 700, 130, 140), MEDIUM_BLUE, 0x0E, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(880, 700, 130, 40), BLUE, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(600, 300, 120, 120), RED, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(650, 300, 130, 130), GREEN, 0x07, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(700, 300, 140, 140), MEDIUM_BLUE, 0x0E, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(750, 300, 130, 130), BLUE, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(900, 20, 120, 130), RED, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(1000, 30, 120, 130), GREEN, 0x07, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(910, 200, 120, 140), MEDIUM_BLUE, 0x0E, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(1000, 180, 140, 120), BLUE, 0x06, 0x01), screen_rect);

    gd.draw(RDPEllipseSC(Rect(1000, 400, 130, 140), RED, 0x0D, 0x00), screen_rect);
    gd.draw(RDPEllipseSC(Rect(1100, 550, 140, 130), BLUE, 0x0D, 0x00), screen_rect);

    gd.draw(RDPEllipseSC(Rect(1030, 430, 65, 70), RED, 0x0D, 0x00), screen_rect);

    // binary raster operations

    gd.draw(RDPEllipseSC(Rect(300, 10, 30, 40), RED, 0x02, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(300, 55, 30, 40), RED, 0x03, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(300, 105, 30, 40), RED, 0x04, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(300, 155, 30, 40), RED, 0x05, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(350, 10, 30, 40), RED, 0x08, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(350, 55, 30, 40), RED, 0x09, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(350, 105, 30, 40), RED, 0x0A, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(350, 155, 30, 40), RED, 0x0C, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(400, 10, 30, 40), RED, 0x0F, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(400, 55, 30, 40), RED, 0x01, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(400, 105, 30, 40), RED, 0x10, 0x01), screen_rect);
    gd.draw(RDPEllipseSC(Rect(400, 105, 30, 40), RED, 0x10, 0x0B), screen_rect);
    gd.draw(RDPEllipseSC(Rect(400, 155, 30, 40), RED, 0x06, 0x01), screen_rect);

    elapusec = ustime() - usec;
    elapcyc = rdtsc() - cycles;

    LOG(LOG_INFO, "elapsed time = %llu %llu %f\n", elapusec, elapcyc, (double)elapcyc / (double)elapusec);

    char message[1024];
    if (!check_sig(gd.drawable, message,
                   "\xa7\xa0\x72\x43\x8a\x05\x86\xc7\xdd\xf6"
                   "\x38\xc1\x7e\xa4\x9d\x20\x2a\x39\xdf\x4e"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    // save_to_png("/tmp/test_ellipse_001.png", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestPatBlt)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
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

    char message[1024];
    if (!check_sig(gd.drawable, message,
        "\x87\x16\x73\x28\x21\x64\x9a\x4a\xea\x25\x60\xe5\x40\x32\x6e\xac\x28\x63\xe5\xad"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
//       dump_png("/tmp/test_patblt_000_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestDestBlt)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
//    gd.draw(RDPPatBlt(screen_rect, 0xFF, WHITE, WHITE, RDPBrush()), screen_rect);
    gd.draw(RDPDestBlt(screen_rect, 0xFF), screen_rect); // WHITENESS
    gd.draw(RDPDestBlt(screen_rect.shrink(5), 0x00), screen_rect); // BLACKNESS
    gd.draw(RDPOpaqueRect(screen_rect.shrink(10), RED), screen_rect); // RED
     // RED inverted becomes CYAN
    gd.draw(RDPDestBlt(screen_rect.shrink(15), 0x55), screen_rect);

    char message[1024];
    if (!check_sig(gd.drawable, message,
    "\x5b\x24\xc7\xec\x13\x7f\xf9\x8a\x32\x59\x62\x50\xef\x6b\x37\x1f\x15\x14\xfc\xbb"
    )){
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
    RDPDrawable gd(width, height);
    gd.draw(RDPOpaqueRect(screen_rect, RED), screen_rect); // RED
    gd.drawable.trace_mouse(100, 100);

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
        "\x75\xc6\xe6\x3b\xd3\x22\x88\x14\x27\x03\xf3\x3e\x3c\x90\x5f\xac\xc1\x5c\x61\xa0"
        )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
//  dump_png("/tmp/test_mouse_000_", gd.drawable);

    gd.drawable.clear_mouse();

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
        "\x2b\x74\x99\xee\x6a\x39\x35\x8b\x87\xe3\x61\xa7\x8f\x91\x38\xdd\x72\xb3\x46\x05"
        )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
//  dump_png("/tmp/test_mouse_001_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestAddMouse2)
{
    // Create a simple capture image and dump it to file
    uint16_t width  = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    gd.draw(RDPOpaqueRect(screen_rect, BLACK), screen_rect); // BLACK
    gd.drawable.trace_mouse(638, 470);

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
        "\xd1\x1b\xe6\x6b\x0a\x66\x87\xd2\x06\x07\x5a\x52\x90\x8a\x37\xc7\x8c\x46\x46\x4b"
        )) {
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    // dump_png("test_mouse2_visible_", gd.drawable);

    gd.drawable.clear_mouse();

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
        "\xf9\x71\xf3\x63\x57\xcc\x45\x41\x40\x90\xce\xce\xce\x55\xa9\x1e\xe1\x9a\xab\x29"
        )) {
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    // dump_png("test_mouse2_clear_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestAddMouse3)
{
    // Create a simple capture image and dump it to file
    uint16_t width  = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    gd.drawable.mouse_hotspot_x = 8;
    gd.drawable.mouse_hotspot_y = 8;
    gd.draw(RDPOpaqueRect(screen_rect, RED), screen_rect); // RED
    gd.drawable.trace_mouse(0, 0);

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
        "\xec\x2b\xf0\xb0\xe0\x8a\x60\x64\xba\x8d\x2d\xbb\x33\xc7\x58\xd0\x4b\x19\x21\x3f"
        )) {
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    // dump_png("test_mouse3_visible_", gd.drawable);

    gd.drawable.clear_mouse();

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
        "\x2b\x74\x99\xee\x6a\x39\x35\x8b\x87\xe3\x61\xa7\x8f\x91\x38\xdd\x72\xb3\x46\x05"
        )) {
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    // dump_png("test_mouse3_clear_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestTimestampMouse)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
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
        "\x0d\x64\x40\x8c\xcb\x82\xd6\x29\x9b\x55\x83\x87\x3d\xd9\x69\xb6\xd7\x5b\x0d\x3d"
        )){
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
        "\x9c\x75\xcc\x7e\x0e\xa2\x3b\x61\xef\x53\x9a\x64\x66\x06\x57\x05\xa1\xe6\x4f\xf0"
        )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    // uncomment to see result in png file
    //dump_png("/tmp/test_timestamp_001_", gd.drawable);


    gd.drawable.clear_timestamp();

    {
        char message[1024];
        if (!check_sig(gd.drawable, message,
        "\x2b\x74\x99\xee\x6a\x39\x35\x8b\x87\xe3\x61\xa7\x8f\x91\x38\xdd\x72\xb3\x46\x05"
        )){
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
    RDPDrawable gd(width, height);
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

bool test_scrblt2(const uint8_t rop, const int cx, const int cy, const char * name, const char * shasig, char * message){
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    gd.draw(RDPOpaqueRect(Rect(90, 90, 120, 120), RED), screen_rect);
    gd.draw(RDPOpaqueRect(screen_rect, BLUE), Rect(100, 100, 100, 100));
    gd.draw(RDPOpaqueRect(Rect(120, 120, 60, 60), PINK), Rect(100, 100, 100, 100));
    gd.drawable.scrblt(90, 90, Rect(300, 300, 120, 120), 0xCC);
    gd.drawable.scrblt(90, 90, Rect(90 + cx, 90 + cy, 120, 120), rop);

    // uncomment to see result in png file
    // char tmpname[128];
    // sprintf(tmpname, "/tmp/test_scrblt_%s", name);
    // dump_png(tmpname, gd.drawable);

    return check_sig(gd.drawable, message, shasig);
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltDown)
{
    char message[1024];
    int res = test_scrblt2(0x00, 0, 20, "down00",
    "\xf8\xbd\xd7\x1d\x93\x78\x8c\xd9\x7a\x88\x6d\xfe\x52\x71\xe5\xaf\x7d\xba\x61\x46"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

//BOOST_AUTO_TEST_CASE(TestDrawableScrBltDown)
//{
//    test_scrblt(0x00, 0, 20, "down00",
//    "\x3c\x39\xae\x2b\x84\x5b\xc6\xa8\x75\xc1\xaf\xbb\x5c\x26\xa9\x1f\x94\x24\xc4\x68");
//}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltRight)
{
    char message[1024];
    int res = test_scrblt2(0x00, 20, 0, "right00",
    "\x76\xbb\x56\xf5\x70\xec\x7e\x19\xc7\x68\xe6\x32\xb3\x43\xf1\xc8\xf1\x78\x6e\xf1"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeft)
{
    char message[1024];
    int res = test_scrblt2(0x00, -20, 0, "left00",
    "\x05\xdf\xba\x3b\x9f\xa9\x5d\x1c\xa9\x12\xa0\x0b\x1d\x10\x26\x68\x41\xc7\x73\xd9"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltUp)
{
    char message[1024];
    int res = test_scrblt2(0x00, 0, -20, "up00",
    "\x55\x73\x7e\xd8\x0a\x36\xde\x1c\x87\xb3\xbb\x78\x6c\xaf\xb2\xcf\x53\xab\xa2\xe6"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeftUp)
{
    char message[1024];
    int res = test_scrblt2(0x00, -20, -20, "left_up00",
    "\xb6\x9a\xe7\xd0\x97\xe1\x3b\xce\x8d\xef\x73\x43\xd2\x50\xba\xd0\x06\xe1\x6c\xca"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltDown11)
{
    char message[1024];
    int res = test_scrblt2(0x11, 0, 20, "down11",
    "\x20\xd5\x27\xaa\x91\xff\xa5\x21\x91\xe8\x94\x08\x6d\x7f\xb3\x52\x38\x62\x96\x2b"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltRight11)
{
    char message[1024];
    int res = test_scrblt2(0x11, 20, 0, "right11",
    "\x85\x82\xdd\x28\x3c\x75\x9c\xdb\xc3\x94\xc8\xd3\x67\x7e\xdf\x76\xfb\x74\x84\x30"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeft11)
{
    char message[1024];
    int res = test_scrblt2(0x11, -20, 0, "left11",
    "\x44\xb8\x04\x46\xf6\x80\x67\x05\x5c\xaf\xb4\xd1\xa3\xcc\x56\x9c\xd6\x97\x1f\x67"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltUp11)
{
    char message[1024];
    int res = test_scrblt2(0x11, 0, -20, "up11",
    "\x8e\xdb\x52\x5b\x59\x6f\xfc\x9c\x70\x6e\x1f\x4d\x73\xad\x46\xa8\x01\xea\xe3\x4d"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestDrawableScrBltLeftUp11)
{
    char message[1024];
    int res = test_scrblt2(0x11, -20, -20, "left_up11",
    "\x5a\xa8\xea\x06\xa6\xa3\xa0\x57\x76\xcb\xc0\xb5\xc3\xb7\x53\x5e\x2b\x7f\xad\x9c"
    , message);
    if (!res){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestMemblt)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    BGRPalette palette;
    init_palette332(palette);

    RDPDrawable gd(width, height);
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
    "\x98\x6c\x40\x0b\x3a\xbc\x39\x38\x29\x11\x77\x37\x98\xe2\x27\xb2\xcb\x61\xec\x5d"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    //dump_png("./test_memblt_", gd.drawable);
}

BOOST_AUTO_TEST_CASE(TestBgr2RGB)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    BGRPalette palette;
    init_palette332(palette);

    RDPDrawable gd(width, height);
    gd.draw(RDPOpaqueRect(screen_rect, 0xFF0000), screen_rect);
    gd.drawable.bgr2rgb();

    char message[1024];
    if (!check_sig(gd.drawable, message,
    "\x2b\x74\x99\xee\x6a\x39\x35\x8b\x87\xe3\x61\xa7\x8f\x91\x38\xdd\x72\xb3\x46\x05"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    //dump_png("./testBGR2RGB", gd.drawable);
}
