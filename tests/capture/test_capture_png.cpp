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
#define BOOST_TEST_MODULE TestPNGCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "test_orders.hpp"

#include "transport.hpp"
#include "testtransport.hpp"
#include "outfilenametransport.hpp"
#include "outfiletransport.hpp"
#include "image_capture.hpp"
#include "staticcapture.hpp"
#include "constants.hpp"
#include "RDP/caches/bmpcache.hpp"
#include <png.h>

    const char expected_red[] = 
    /* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    /* 0000 */ "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    /* 0000 */ "\x00\x00\x03\x20\x00\x00\x02\x58\x08\x02\x00\x00\x00"             //... ...X.....
    /* 0000 */ "\x15\x14\x15\x27"                                                 //...'
    /* 0000 */ "\x00\x00\x0a\xa9\x49\x44\x41\x54"                                 //....IDAT
    /* 0000 */ "\x78\x9c\xed\xd6\xc1\x09\x00\x20\x10\xc0\x30\x75\xff\x9d\xcf\x25" //x...... ..0u...%
    /* 0010 */ "\x0a\x82\x24\x13\xf4\xd9\x3d\x0b\x00\x80\xd2\x79\x1d\x00\x00\xf0" //..$...=....y....
    /* 0020 */ "\x1b\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //.....3X..1....3X
    /* 0030 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0040 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0050 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0060 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0070 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0080 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0090 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 00a0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 00b0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 00c0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 00d0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 00e0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 00f0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0100 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0110 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0120 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0130 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0140 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0150 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0160 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0170 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0180 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0190 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 01a0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 01b0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 01c0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 01d0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 01e0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 01f0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0200 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0210 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0220 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0230 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0240 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0250 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0260 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0270 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0280 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0290 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 02a0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 02b0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 02c0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 02d0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 02e0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 02f0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0300 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0310 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0320 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0330 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0340 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0350 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0360 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0370 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0380 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0390 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 03a0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 03b0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 03c0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 03d0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 03e0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 03f0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0400 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0410 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0420 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0430 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0440 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0450 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0460 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0470 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0480 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0490 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 04a0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 04b0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 04c0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 04d0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 04e0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 04f0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0500 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0510 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0520 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0530 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0540 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0550 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0560 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0570 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0580 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0590 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 05a0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 05b0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 05c0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 05d0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 05e0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 05f0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0600 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0610 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0620 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0630 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0640 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0650 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0660 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0670 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0680 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0690 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 06a0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 06b0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 06c0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 06d0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 06e0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 06f0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0700 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0710 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0720 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0730 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0740 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0750 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0760 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0770 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0780 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0790 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 07a0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 07b0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 07c0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 07d0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 07e0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 07f0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0800 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0810 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0820 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0830 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0840 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0850 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0860 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0870 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0880 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0890 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 08a0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 08b0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 08c0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 08d0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 08e0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 08f0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0900 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0910 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0920 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0930 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0940 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0950 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0960 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0970 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0980 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0990 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 09a0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 09b0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 09c0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 09d0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 09e0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 09f0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0a00 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0a10 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0a20 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0a30 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0a40 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0a50 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0a60 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0a70 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0a80 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0a90 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0aa0 */ "\x58\x00\x00\xb1\x0b\xbb\xfd\x05\xaf"                             //X........
    /* 0000 */ "\x0d\x9d\x5e\xa4"                                                 //..^.
    /* 0000 */ "\x00\x00\x00\x00\x49\x45\x4e\x44"                                 //....IEND
     /* 0000 */ "\xae\x42\x60\x82"                                                 //.B`.
    ;

BOOST_AUTO_TEST_CASE(TestTransportPngOneRedScreen)
{
    // This is how the expected raw PNG (a big flat RED 800x600 screen)
    // will look as a string

    RDPDrawable d(800, 600, RDPDrawableConfig(true));
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RED);
    d.draw(cmd, screen_rect);
    TestTransport trans("TestTransportPNG", "", 0, expected_red, sizeof(expected_red)-1);;
    transport_dump_png24(&trans, d.drawable.data, 800, 600, d.drawable.rowsize);
}

BOOST_AUTO_TEST_CASE(TestImageCapturePngOneRedScreen)
{
    CheckTransport trans(expected_red, sizeof(expected_red)-1);;
    ImageCapture d(trans, 800, 600);
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RED);
    d.draw(cmd, screen_rect);
    d.flush();
}

BOOST_AUTO_TEST_CASE(TestImageCaptureToFilePngOneRedScreen)
{
    const char * filename = "test.png";
    size_t len = strlen(filename);
    char path[1024];
    memcpy(path, filename, len);
    path[len] = 0;
    int fd = ::creat(path, 0777);
    if (fd == -1){
        LOG(LOG_INFO, "OutByFilename transport write failed with error : %s on %s", strerror(errno), path);
        BOOST_CHECK(false);
        return;
    }

    OutFileTransport trans(fd);
    ImageCapture d(trans, 800, 600);
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RED);
    d.draw(cmd, screen_rect);
    d.flush();
    rio_clear(&trans.rio); // close file before checking size
    BOOST_CHECK_EQUAL(2786, filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestImageCaptureToFilePngBlueOnRed)
{
    const int groupid = 0;
    OutFilenameTransport trans(SQF_PATH_FILE_PID_COUNT_EXTENSION, "./", "test", ".png", groupid);
    ImageCapture d(trans, 800, 600);
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RED);
    d.draw(cmd, screen_rect);
    d.flush();

    BOOST_CHECK_EQUAL(2786, sq_outfilename_filesize(&trans.seq, 0));
    sq_outfilename_unlink(&trans.seq, 0);

    RDPOpaqueRect cmd2(Rect(50, 50, 100, 50), BLUE);
    d.draw(cmd2, screen_rect);
    trans.next();
    d.flush();

    BOOST_CHECK_EQUAL(2806, sq_outfilename_filesize(&trans.seq, 1));
    sq_outfilename_unlink(&trans.seq, 1);
}


BOOST_AUTO_TEST_CASE(TestOneRedScreen)
{
    struct timeval now;
    now.tv_sec = 1000;
    now.tv_usec = 0;

    Rect screen_rect(0, 0, 800, 600);
    const int groupid = 0;
    OutFilenameTransport trans(SQF_PATH_FILE_PID_COUNT_EXTENSION, "./", "xxxtest", ".png", groupid);
    SQ * seq = &(trans.seq);
    Inifile ini;
    ini.globals.png_interval = 1;
    ini.globals.png_limit = 3;
    StaticCapture consumer(now, trans, &(trans.seq), 800, 600, ini);

    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RED);
    consumer.draw(cmd, screen_rect);

    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 0));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 1));
    

    now.tv_sec++; consumer.snapshot(now, 0, 0, true, true);

    BOOST_CHECK_EQUAL(3051, sq_outfilename_filesize(seq, 0));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 1));

    now.tv_sec++; consumer.snapshot(now, 0, 0, true, true);

    BOOST_CHECK_EQUAL(3051, sq_outfilename_filesize(seq, 0));
    BOOST_CHECK_EQUAL(3065, sq_outfilename_filesize(seq, 1));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 2));

    now.tv_sec++; consumer.snapshot(now, 0, 0, true, true);

    BOOST_CHECK_EQUAL(3051, sq_outfilename_filesize(seq, 0));
    BOOST_CHECK_EQUAL(3065, sq_outfilename_filesize(seq, 1));
    BOOST_CHECK_EQUAL(3064, sq_outfilename_filesize(seq, 2));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 3));

    now.tv_sec++; consumer.snapshot(now, 0, 0, true, true);

    rio_clear(&trans.rio);
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 0));
    BOOST_CHECK_EQUAL(3065, sq_outfilename_filesize(seq, 1));
    BOOST_CHECK_EQUAL(3064, sq_outfilename_filesize(seq, 2));
    BOOST_CHECK_EQUAL(3054, sq_outfilename_filesize(seq, 3));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 4));

    ini.globals.png_limit = 10;
    consumer.update_config(ini);

    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 0));
    BOOST_CHECK_EQUAL(3065, sq_outfilename_filesize(seq, 1));
    BOOST_CHECK_EQUAL(3064, sq_outfilename_filesize(seq, 2));
    BOOST_CHECK_EQUAL(3054, sq_outfilename_filesize(seq, 3));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 4));

    ini.globals.png_limit = 2;
    consumer.update_config(ini);

    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 0));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 1));
    BOOST_CHECK_EQUAL(3064, sq_outfilename_filesize(seq, 2));
    BOOST_CHECK_EQUAL(3054, sq_outfilename_filesize(seq, 3));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 4));

    ini.globals.png_limit = 0;
    consumer.update_config(ini);

    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 1));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 2));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 3));
    BOOST_CHECK_EQUAL(-1, sq_outfilename_filesize(seq, 4));
}

BOOST_AUTO_TEST_CASE(TestSmallImage)
{
    const int groupid = 0;
    OutFilenameTransport trans(SQF_PATH_FILE_PID_COUNT_EXTENSION, "./", "sample", ".png", 0, 0x100);
    Rect scr(0, 0, 20, 10);
    ImageCapture d(trans, scr.cx, scr.cy);
    d.draw(RDPOpaqueRect(scr, RED), scr);
    d.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr);
    d.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr);
    d.flush();
    BOOST_CHECK_EQUAL(107, sq_outfilename_filesize(&(trans.seq), 0));
    sq_outfilename_unlink(&(trans.seq), 0);
}



BOOST_AUTO_TEST_CASE(TestScaleImage)
{
    const int width = 800;
    const int height = 600;
    const int groupid = 0;
    OutFilenameTransport trans(SQF_PATH_FILE_PID_COUNT_EXTENSION, "./", "test_scale", ".png", groupid);
    Rect scr(0, 0, width, height);
    ImageCapture d(trans, scr.cx, scr.cy);
    d.zoom(50);

    {
        const char * filename = "./tests/fixtures/win2008capture10.png";
        FILE * fd = fopen(filename, "r");
        TODO("Add ability to write image to file or read image from file in RDPDrawable")
        read_png24(fd, d.drawable.data, d.drawable.width, d.drawable.height, d.drawable.rowsize);
        fclose(fd);
    }
    d.flush();
    BOOST_CHECK_EQUAL(8176, sq_outfilename_filesize(&(trans.seq), 0));
    sq_outfilename_unlink(&(trans.seq), 0);
}


BOOST_AUTO_TEST_CASE(TestBogusBitmap)
{
    BOOST_CHECK(1);
    const int groupid = 0;    
    OutFilenameTransport trans(SQF_PATH_FILE_PID_COUNT_EXTENSION, "./", "bogus", ".png", 0, 0x100);
    Rect scr(0, 0, 800, 600);
    ImageCapture d(trans, scr.cx, scr.cy);
    d.draw(RDPOpaqueRect(scr, GREEN), scr);

    uint8_t source64x64[] = {
// MIX_SET 60 remaining=932 bytes pix=0
/* 0000 */ 0xc0, 0x2c, 0xdf, 0xff,                                      // .,..
// COPY 6 remaining=931 bytes pix=60
/* 0000 */ 0x86, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,           // .............
// FOM_SET 3006 remaining=913 bytes pix=66
/* 0000 */ 0xf7, 0xbe, 0x0b, 0x01, 0xF0, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // ................
/* 0010 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0020 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0030 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0040 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0050 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0060 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0070 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0080 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0090 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 00a0 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 00b0 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 00c0 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 00d0 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 00e0 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 00f0 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0100 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0110 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0120 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0130 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0140 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0150 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0160 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0170 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x04,           // ....QDDDDDDD.

// BICOLOR 64 remaining=532 bytes pix=3072
/* 0000 */ 0xe0, 0x10, 0x55, 0xad, 0x35, 0xad,                                // ..U.5.
// COPY 63 remaining=530 bytes pix=3136
/* 0000 */ 0x80, 0x1f, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0010 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0020 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0030 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0040 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0050 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0060 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0070 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
// COLOR 65 remaining=400 bytes pix=3199
/* 0000 */ 0x60, 0x21, 0xdf, 0xff,                                      // `!..
// FOM 387 remaining=396 bytes pix=3264
/* 0000 */ 0xf2, 0x83, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // ................
/* 0010 */ 0x11, 0x11, 0x11, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ...DDDDDDDDDDDDD
/* 0020 */ 0x44, 0x44, 0x44, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDD.............
/* 0030 */ 0x11, 0x11, 0x11, 0x04,                                      // ....
// COPY 3 remaining=347 bytes pix=3651
/* 0000 */ 0x83, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff,                             // .......
// FOM 122 remaining=338 bytes pix=3654
/* 0000 */ 0x40, 0x79, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // @y..............
/* 0010 */ 0x11, 0x01,                                            // ..
// COPY 99 remaining=321 bytes pix=3776
/* 0000 */ 0x80, 0x43, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .C..............
/* 0010 */ 0xdf, 0xff, 0xde, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0x00, 0x00,  // ................
/* 0020 */ 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................
/* 0030 */ 0xdf, 0xff, 0xde, 0xff, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0040 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0050 */ 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0060 */ 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0070 */ 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0080 */ 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0090 */ 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff,  // ................
/* 00a0 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00,  // ................
/* 00b0 */ 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00,  // ................
/* 00c0 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00,                          // ........
// FILL 9 remaining=122 bytes pix=3875
/* 0000 */ 0x09,                                               // .
// FOM 63 remaining=119 bytes pix=3884
/* 0000 */ 0x40, 0x3e, 0x11, 0x11, 0x41, 0x44, 0x04, 0x40, 0x00, 0x44,                    // @>..AD.@.D
// COPY 15 remaining=111 bytes pix=3947
/* 0000 */ 0x8f, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde,  // ................
/* 0010 */ 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,     // ...............
// FOM 24 remaining=79 bytes pix=3962
/* 0000 */ 0x43, 0x11, 0x11, 0x11,                                      // C...
// COPY 8 remaining=76 bytes pix=3986
/* 0000 */ 0x88, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf,  // ................
/* 0010 */ 0xff,                                               // .
// FOM 57 remaining=57 bytes pix=3994
/* 0000 */ 0x40, 0x38, 0x01, 0x10, 0x11, 0x11, 0x51, 0x44, 0x44, 0x00,                    // @8....QDD.
// COPY 3 remaining=49 bytes pix=4051
/* 0000 */ 0x83, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff,                             // .......
// FILL 10 remaining=42 bytes pix=4054
/* 0000 */ 0x0a,                                               // .
// FILL 12 remaining=41 bytes pix=4064
// magic mix

/* 0000 */ 0x0c,                                               // .
// COPY 20 remaining=40 bytes pix=4075
/* 0000 */ 0x94, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf,  // ................
/* 0010 */ 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf,  // ................
/* 0020 */ 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,                       // .........
    }
    ;

    Bitmap bloc64x64(16, NULL, 64, 64, source64x64, sizeof(source64x64), true );
    d.draw(RDPMemBlt(0, Rect(100, 100, bloc64x64.cx, bloc64x64.cy), 0xCC, 0, 0, 0), scr, bloc64x64);

    Bitmap good16(24, bloc64x64);
    d.draw(RDPMemBlt(0, Rect(200, 200, good16.cx, good16.cy), 0xCC, 0, 0, 0), scr, good16);
    
    BStream stream(8192);
    good16.compress(stream);
    stream.mark_end();
    Bitmap bogus(24, NULL, 64, 64, stream.data, stream.size(), true);
    d.draw(RDPMemBlt(0, Rect(300, 100, bogus.cx, bogus.cy), 0xCC, 0, 0, 0), scr, bogus);

    d.flush();
    BOOST_CHECK_EQUAL(4094, sq_outfilename_filesize(&(trans.seq), 0));
    sq_outfilename_unlink(&(trans.seq), 0);
}


BOOST_AUTO_TEST_CASE(TestBogusBitmap2)
{
    BOOST_CHECK(1);
    const int groupid = 0;    
    OutFilenameTransport trans(SQF_PATH_FILE_PID_COUNT_EXTENSION, "./", "bogus", ".png", groupid, 0x100);
    Rect scr(0, 0, 800, 600);
    ImageCapture d(trans, scr.cx, scr.cy);
    d.draw(RDPOpaqueRect(scr, GREEN), scr);

    uint8_t source32x1[] = 
//MemBlt Primary Drawing Order (0x0D)
//memblt(id=0 idx=15 x=448 y=335 cx=32 cy=1)
//Cache Bitmap V2 (Compressed) Secondary Drawing Order (0x05)
//update_read_cache_bitmap_v2_order
//update_read_cache_bitmap_v2_order id=0
//update_read_cache_bitmap_v2_order flags=8 CBR2_NO_BITMAP_COMPRESSION_HDR
//update_read_cache_bitmap_v2_order width=32 height=1
//update_read_cache_bitmap_v2_order Index=16
//rledecompress width=32 height=1 cbSrcBuffer=58

//-- /* BMP Cache compressed V2 */
//-- COPY1 5
//-- MIX 1
//-- COPY1 7
//-- MIX 1
//-- COPY1 2
//-- MIX_SET 4
//-- COPY1 9
//-- MIX_SET 3

           "\x85\xf8\xff\x2b\x6a\x6c\x12\x8d\x12\x79\x14"
           "\x21"
           "\x87\x15\xff\x2b\x42\x4b\x12\x4c\x12\x6c\x12\x4c\x12\x38\x14"
           "\x21"
           "\x82\x32\xfe\x6c\x12"
           "\xc4\x8d\x12"
           "\x89\x6d\x12\x4c\x12\x1f\x6e\xff\xff\x2a\xb4\x2b\x12\x6d\x12\xae\x12\xcf\x1a"
           "\xc3\xef\x1a"
    ;

    try {
        Bitmap bloc32x1(16, NULL, 32, 1, source32x1, sizeof(source32x1)-1, true );
        d.draw(RDPMemBlt(0, Rect(100, 100, bloc32x1.cx, bloc32x1.cy), 0xCC, 0, 0, 0), scr, bloc32x1);
    } catch (Error e) {
        printf("exception caught e=%u\n", e.id);
    };

    d.flush();
    BOOST_CHECK_EQUAL(2913, sq_outfilename_filesize(&(trans.seq), 0));
    sq_outfilename_unlink(&(trans.seq), 0);
}

