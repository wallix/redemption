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
#define BOOST_TEST_MODULE TestStaticCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "test_orders.hpp"
#include "transport.hpp"
#include "image_capture.hpp"
#include "staticcapture.hpp"
#include "constants.hpp"
#include "RDP/caches/bmpcache.hpp"
#include <png.h>


BOOST_AUTO_TEST_CASE(TestOneRedScreen)
{
    Rect screen_rect(0, 0, 800, 600);
    FileSequence sequence("path file pid count extension", "./", "test", "png");
    OutByFilenameSequenceTransport trans(sequence);

    struct timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;
    
    Inifile ini;
    ini.globals.png_limit = 3;
    ini.globals.png_interval = 20;
    StaticCapture consumer(now, trans, sequence, 800, 600, ini);

    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RED);
    consumer.draw(cmd, screen_rect);
    consumer.snapshot(now, 10, 10, true, false);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, true, false);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, true, false);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, true, false);
    now.tv_sec++;

    RDPOpaqueRect cmd1(Rect(100, 100, 200, 200), BLUE);
    consumer.draw(cmd1, screen_rect);
    consumer.snapshot(now, 10, 10, true, false);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, true, false);
    now.tv_sec++;
    ::close(trans.fd);

    BOOST_CHECK_EQUAL(3092, sequence.filesize(0));
    BOOST_CHECK_EQUAL(3108, sequence.filesize(1));
    sequence.unlink(0);
    sequence.unlink(1);
}

