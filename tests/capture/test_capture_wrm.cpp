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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include <sys/time.h>

#include "test_orders.hpp"
#include "transport.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"
#include "image_capture.hpp"
#include "constants.hpp"

const char expected_stripped_wrm[] = 
/* 0000 */ "\xEE\x03\x10\x00\x01\x00\x00\x00" // 03EE: META 0010: chunk_len=16 0001: 1 order 0000:flags=0
           "\x20\x03\x58\x02\x18\x00\x00\x00" // width = 800, height=600, bpp=24 PAD: 2 bytes
/* 0000 */ "\xf0\x03\x10\x00\x01\x00\x00\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order 0000:flags=0
/* 0000 */ "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x3BAA0C40 = 1001000000
/* 0000 */ "\x00\x00\x1A\x00\x02\x00\x00\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders 0000:flags=0
/* 0000 */ "\x09\x0a\x2c\x20\x03\x58\x02\xff"         // Green Rect(0, 0, 800, 600)
           "\x01\x6e\x32\x00\xbc\x02\x1e\x00\x00\xff" // Blue  Rect(0, 50, 700, 80)
           "\xf0\x03\x10\x00\x01\x00\x00\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order 0000:flags=0
           "\xc0\x99\x05\x3c\x00\x00\x00\x00" // 0x3C0599C0 = 1007000000
           "\x00\x00\x12\x00\x02\x00\x00\x00" // 0000: ORDERS  0012:chunk_len=18 0002: 2 orders 0000:flags=0
           "\x11\x32\x32\xff\xff\x11\x62\x32\x00\x00" // White rect(0, 100, 700, 130), red rect(0, 150, 700, 180)
    ;

BOOST_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrm)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    BStream stream(65536);
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1, 511);
    Inifile ini;
    GraphicsToFile consumer(&trans, &stream, &ini, 24, 600, 256, 300, 1024, 262, 4096, now);
    RDPOpaqueRect cmd0(Rect(0, 0, 800, 600), GREEN);
    consumer.draw(cmd0, screen_rect);
    now.tv_sec++;
    consumer.timestamp(now);

    RDPOpaqueRect cmd1(Rect(0, 50, 700, 30), BLUE);
    consumer.draw(cmd1, screen_rect);
    consumer.flush();

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    RDPOpaqueRect cmd2(Rect(0, 100, 700, 30), WHITE);
    consumer.draw(cmd2, screen_rect);
    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), RED);
    consumer.draw(cmd3, screen_rect);
    now.tv_sec++;
    consumer.timestamp(now);
 
    consumer.flush();
}

BOOST_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrmReplay2)
{
    // Same as above, show timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    BStream stream(65536);
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1, 511);
    Inifile ini;
    GraphicsToFile consumer(&trans, &stream, &ini, 24, 600, 256, 300, 1024, 262, 4096, now);

    RDPOpaqueRect cmd0(Rect(0, 0, 800, 600), GREEN);
    consumer.draw(cmd0, screen_rect);
    RDPOpaqueRect cmd1(Rect(0, 50, 700, 30), BLUE);
    consumer.draw(cmd1, screen_rect);
    now.tv_sec++;
    consumer.timestamp(now);
    consumer.flush();

    RDPOpaqueRect cmd2(Rect(0, 100, 700, 30), WHITE);
    consumer.draw(cmd2, screen_rect);
    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), RED);
    consumer.draw(cmd3, screen_rect);
    now.tv_sec+=6;
    consumer.timestamp(now);
 
    consumer.flush();
}

BOOST_AUTO_TEST_CASE(TestCaptureToWrmReplayToPng)
{
    // Same as above, show timestamps are applied only when flushing
    BOOST_CHECK_EQUAL(0, 0);
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    BStream stream(65536);
    OutByFilenameTransport trans("./testcap.wrm");
    BOOST_CHECK_EQUAL(0, 0);
    Inifile ini;
    GraphicsToFile consumer(&trans, &stream, &ini, 24, 600, 256, 300, 1024, 262, 4096, now);
    BOOST_CHECK_EQUAL(0, 0);
    RDPOpaqueRect cmd0(Rect(0, 0, 800, 600), GREEN);
    consumer.draw(cmd0, screen_rect);
    RDPOpaqueRect cmd1(Rect(0, 50, 700, 30), BLUE);
    consumer.draw(cmd1, screen_rect);
    now.tv_sec++;
    BOOST_CHECK_EQUAL(0, 0);
    consumer.timestamp(now);
    consumer.flush();
    BOOST_CHECK_EQUAL(0, 0);

    RDPOpaqueRect cmd2(Rect(0, 100, 700, 30), WHITE);
    consumer.draw(cmd2, screen_rect);
    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), RED);
    consumer.draw(cmd3, screen_rect);
    now.tv_sec+=6;
    consumer.timestamp(now);
    consumer.flush();
    BOOST_CHECK_EQUAL(0, 0);
    ::close(trans.fd);
    BOOST_CHECK_EQUAL(92, filesize("./testcap.wrm"));
    
    InByFilenameTransport in_wrm_trans("./testcap.wrm");
    FileSequence sequence("path file pid count extension", "./", "testcap", "png");
    OutByFilenameSequenceTransport out_png_trans(sequence);

    now.tv_sec = 5000;
    FileToGraphic player(&in_wrm_trans, now, screen_rect);
    ImageCapture png_recorder(out_png_trans, 800, 600, true);
    player.add_recorder(&png_recorder);

    // META
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();

    // Timestamp
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Green Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Blue Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Timestamp
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // White Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Red Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    BOOST_CHECK_EQUAL(false, player.next_order());
    ::close(in_wrm_trans.fd);
 
    // clear PNG files   
    size_t sz[6] = {1476, 2786, 2800, 2800, 2814, 2823};
    for (int i = 0; i < 6 ; i++){
        char path[1024];
        BOOST_CHECK_EQUAL(sz[i], sequence.filesize(i));
        sequence.get_name(path, sizeof(path), i);
        ::unlink(path);
    }
   ::unlink("./testcap.wrm");
}
