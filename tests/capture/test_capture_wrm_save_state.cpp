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

   Unit test to state saving code in WRM files

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
/* 0000 */ "\xEE\x03\x1A\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=16 0001: 1 order
           "\x64\x00\x64\x00\x18\x00" // width = 20, height=10, bpp=24 
           "\x02\x00\x00\x01\x02\x00\x00\x04\x02\x00\x00\x10"  // caches sizes

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000

/* 0000 */ "\x00\x00\x2d\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x19\x0a\x4c\x64\x64\xff"         // RED rect  // order 0A=opaque rect
// -----------------------------------------------------
/* 0020 */ "\x03\x09\x00\x00\x04\x02"         // Secondary drawing order header. Order = 02: Compressed bitmap
           "\x01\x00\x14\x0a\x18\x07\x00\x00\x00" // 0x01=cacheId 0x00=pad 0x14=width(20) 0x0A=height(10) 0x18=24 bits
                                                  // 0x0007=bitmapLength 0x0000=cacheIndex
           "\xc0\x04\x00\x00\xff\x00\x94"         // compressed bitamp data (7 bytes)
// -----------------------------------------------------

           "\x59\x0d\x3d\x01\x00\x5a\x14\x0a\xcc" // order=0d : MEMBLT

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x000000003BAA0C40 = 1001000000
           
           "\x00\x00\x1e\x00\x00\x00\x01\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
// -----------------------------------------------------
/* 0000 */ "\x03\x09\x00\x00\x04\x02"
           "\x01\x00\x14\x0a\x18\x07\x00\x00\x00"
           "\xc0\x04\x00\x00\xff\x00\x94"
// -----------------------------------------------------
           ;

BOOST_AUTO_TEST_CASE(TestSaveCache)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 100, 100);
    BStream stream(65536);
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1, 511);
    Inifile ini;
    BmpCache bmp_cache(24, 2, 256, 2, 1024, 2, 4096);
    GraphicToFile consumer(now, &trans, &stream, &ini, scr.cx, scr.cy, 24, bmp_cache);
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(scr, BLUE), scr);
    
    uint8_t comp20x10RED[] = {
        0xc0, 0x04, 0x00, 0x00, 0xFF, // MIX 20 (0, 0, FF) 
        0x00, 0x94                    // FILL 9 * 20
    };

    Bitmap bloc20x10(24, NULL, 20, 10, comp20x10RED, sizeof(comp20x10RED), true );
    consumer.draw(
        RDPMemBlt(0, Rect(0, scr.cy - 10, bloc20x10.cx, bloc20x10.cy), 0xCC, 0, 0, 0),
        scr, 
        bloc20x10);
    consumer.flush();
        
    now.tv_sec++;
    consumer.timestamp(now);

    consumer.save_bmp_caches();
    
    consumer.flush();
}

