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
#define BOOST_TEST_MODULE TestNativeCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "test_orders.hpp"
#include "transport.hpp"
#include "outbyfilenamesequencetransport.hpp"
#include "image_capture.hpp"
#include "nativecapture.hpp"
#include "constants.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "config.hpp"
#include <png.h>


BOOST_AUTO_TEST_CASE(TestSimpleBreakpoint)
{
    Rect scr(0, 0, 800, 600);
    FileSequence sequence("path file pid count extension", "./", "test", ".wrm");
    OutByFilenameSequenceTransport trans(sequence);

    struct timeval now;
    now.tv_sec = 1000;
    now.tv_usec = 0;
    
    BmpCache bmp_cache(24, 600, 768, 300, 3072, 262, 12288); 
    Inifile ini;
    RDPDrawable drawable(800, 600, true);
    NativeCapture consumer(now, trans, 800, 600, bmp_cache, &drawable, ini);
    ini.globals.frame_interval = 100; // one snapshot by second
    ini.globals.break_interval = 5;   // one WRM file every 5 seconds
    consumer.update_config(ini);

    consumer.draw(RDPOpaqueRect(scr, RED), scr);
    consumer.snapshot(now, 10, 10, true, false);
    now.tv_sec += 6;
    consumer.snapshot(now, 10, 10, true, false);
    ::close(trans.fd);
    
    BOOST_CHECK_EQUAL((unsigned)1544, (unsigned)sequence.filesize(0));
    sequence.unlink(0);
    BOOST_CHECK_EQUAL((unsigned)3254, (unsigned)sequence.filesize(1));
    sequence.unlink(1);
}

