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

#define LOGNULL
#include <sys/time.h>

#include "test_orders.hpp"
#include "transport.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"
#include "GraphicToFile.hpp"
#include "image_capture.hpp"
#include "constants.hpp"

BOOST_AUTO_TEST_CASE(TestSplittedCapture)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 800, 600);
    
    Inifile ini;
    BmpCache bmp_cache(24, 600, 256, 300, 1024, 262, 4096);
    FileSequence wrm_sequence("path file pid count extension", "./", "capture", "wrm");
    OutByFilenameSequenceTransport out_wrm_trans(wrm_sequence);

    GraphicToFile consumer(now, &out_wrm_trans, ini, scr.cx, scr.cy, 24, bmp_cache);
    consumer.draw(RDPOpaqueRect(scr, GREEN), scr);
    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), BLUE), scr);
    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), WHITE), scr);
    consumer.draw(RDPOpaqueRect(Rect(0, 150, 700, 30), RED), scr);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 10), BLACK), scr);
    consumer.flush();
}


