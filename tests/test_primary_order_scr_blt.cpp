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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TestOrderScrBlt
#include <boost/test/auto_unit_test.hpp>
#include <algorithm>

#include "NewRDPOrders.hpp"

#include "./test_orders.hpp"

BOOST_AUTO_TEST_CASE(TestScrBlt)
{
    using namespace RDP;

    RDPScrBlt scrblt_ref(Rect(0, 0, 10, 10), 0, 0, 0);

    {
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(0, 0, 10, 10), 0, 0, 0);

        RDPOrderCommon newcommon(SCREENBLT, Rect(10, 10, 800, 600));
        RDPScrBlt(Rect(100, 150, 50, 60), 0xFF, 300, 400).emit(stream, newcommon, state_common, state_scrblt);

        uint8_t datas[16] = {CHANGE | STANDARD, SCREENBLT,
            0x7F,   // x, y, cx, cy, rop, srcx, srcy coordinates changed
            100, 0, // x = 100
            150, 0, // y = 150
            50,  0, // cx = 50
            60,  0, // cy = 60
            0xFF,   // rop = 0xFF
            0x2C, 1,  // srcx = 300
            0x90, 1,  // srcy = 400
        };
        check_datas(stream.p-stream.data, stream.data, 16, datas, "ScrBlt 1");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)SCREENBLT, common_cmd.order);

        RDPScrBlt cmd(Rect(0, 0, 10, 10), 0, 0, 0);
        cmd.receive(stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(0, 0, 800, 600)),
            RDPScrBlt(Rect(100, 150, 50, 60), 0xFF, 300, 400),
            "ScrBlt 1");
    }

    {
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(0, 0, 10, 10), 0, 0, 0);

        RDPOrderCommon newcommon(SCREENBLT, Rect(10, 10, 800, 600));
        RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150).emit(stream, newcommon, state_common, state_scrblt);

        uint8_t datas[16] = {
            CHANGE | STANDARD,
            SCREENBLT,
            0x7F,   // x, y, cx, cy, rop, srcx, srcy coordinates changed
            0x2C, 1, // x = 300
            0x90, 1, // y = 400
            50,  0, // cx = 50
            60,  0, // cy = 60
            0xFF,   // rop = 0xFF
            100, 0,  // srcx = 100
            150, 0,  // srcy = 150
        };
        check_datas(stream.p-stream.data, stream.data, 16, datas, "ScrBlt 2");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)SCREENBLT, common_cmd.order);

        RDPScrBlt cmd(Rect(0, 0, 10, 10), 0, 0, 0);
        cmd.receive(stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(0, 0, 800, 600)),
            RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150),
            "ScrBlt 2");

    }

    {
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(310, 390, 10, 10), 0xFF, 110, 140);

        RDPOrderCommon newcommon(SCREENBLT, Rect(10, 10, 800, 600));
        RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150).emit(stream, newcommon, state_common, state_scrblt);

        uint8_t datas[9] = {
            CHANGE | STANDARD | DELTA,
            SCREENBLT,
            0x6F,   // x, y, cx, cy, srcx, srcy coordinates changed
            -10,    // x = -10 -> 300
            +10,    // y = 390 +10 = 400
            +40,    // cx = 10 + 40 -> 50
            +50,    // cy = 10 + 50 -> 60
            -10,     // srcx = 110 - 10 = 100
            +10,    // srcy = 140 +10 = 150
        };
        check_datas(stream.p-stream.data, stream.data, 9, datas, "ScrBlt 3");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)SCREENBLT, common_cmd.order);

        RDPScrBlt cmd(Rect(310, 390, 10, 10), 0xFF, 110, 140);
        cmd.receive(stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(0, 0, 800, 600)),
            RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150),
            "ScrBlt 3");

    }

    {
        Stream stream(1000);
        RDPOrderCommon state_common(SCREENBLT, Rect(311, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(310, 390, 10, 10), 0xFF, 110, 140);

        RDPOrderCommon newcommon(SCREENBLT, Rect(311, 0, 800, 600));
        RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150).emit(stream, newcommon, state_common, state_scrblt);

        uint8_t datas[8] = {
            STANDARD | BOUNDS | DELTA | LASTBOUNDS,
            0x6F,   // x, y, cx, cy, srcx, srcy coordinates changed
            -10,    // x = -10 -> 300
            +10,    // y = 390 +10 = 400
            +40,    // cx = 10 + 40 -> 50
            +50,    // cy = 10 + 50 -> 60
            -10,     // srcx = 110 - 10 = 100
            +10,    // srcy = 140 +10 = 150
        };
        check_datas(stream.p-stream.data, stream.data, 8, datas, "ScrBlt 4");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)SCREENBLT, common_cmd.order);

        RDPScrBlt cmd(Rect(310, 390, 10, 10), 0xFF, 110, 140);
        cmd.receive(stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(311, 0, 800, 600)),
            RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150),
            "ScrBlt 4");

    }
}
