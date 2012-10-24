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

   Unit test to image chunk in WRM files

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmImageChunk
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include <sys/time.h>

#include "test_orders.hpp"
#include "transport.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"
#include "GraphicToFile.hpp"
#include "image_capture.hpp"
#include "constants.hpp"

const char expected_stripped_wrm[] = 
/* 0000 */ "\xEE\x03\x10\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=16 0001: 1 order
           "\x14\x00\x0A\x00\x18\x00\x00\x00" // width = 20, height=10, bpp=24 PAD: 2 bytes
/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000
/* 0000 */ "\x00\x00\x1e\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x19\x0a\x1c\x14\x0a\xff"             // RED rect
/* 0000 */ "\x11\x5f\x05\x05\xF6\xf9\x00\xFF\x11" // BLUE RECT
/* 0000 */ "\x3f\x05\xfb\xf7\x07\xff\xff"         // WHITE RECT
/* 0000 */ "\x00\x10\x73\x00\x00\x00\x01\x00" // 0x1000: IMAGE_CHUNK 0048: chunk_len=86 0001: 1 order
    "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00"             //.............
    "\x3b\x37\xe9\xb1"                                                 //;7..
    "\x00\x00\x00\x32\x49\x44\x41\x54"                                 //...2IDAT
    "\x28\x91\x63\xfc\xcf\x80\x17\xfc\xff\xcf\xc0\xc8\x88\x4b\x92\x09" //(.c..........K..
    "\xbf\x5e\xfc\x60\x88\x6a\x66\x41\xe3\x33\x32\xa0\x84\xe0\x7f\x54" //.^.`.jfA.32....T
    "\x91\xff\x0c\x28\x81\x37\x70\xce\x66\x1c\xb0\x78\x06\x00\x69\xdc" //...(.7p.f..x..i.
    "\x0a\x12"                                                         //..
    "\x86\x4a\x0c\x44"                                                 //.J.D
    "\x00\x00\x00\x00\x49\x45\x4e\x44"                                 //....IEND
    "\xae\x42\x60\x82"                                                 //.B`.
    ;

BOOST_AUTO_TEST_CASE(TestImageChunk)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 20, 10);
    BStream stream(65536);
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1, 511);
    Inifile ini;
    GraphicToFile consumer(now, &trans, &stream, &ini, scr.cx, scr.cy, 24, 600, 256, 300, 1024, 262, 4096);
    consumer.draw(RDPOpaqueRect(scr, RED), scr);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr);
    consumer.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr);
    consumer.flush();
    consumer.send_image_chunk();
}

