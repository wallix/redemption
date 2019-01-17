/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

#define RED_TEST_MODULE TestOrderMemBlt
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"


RED_AUTO_TEST_CASE(TestMemBlt)
{
    using namespace RDP;

    {
        StaticOutStream<1000> out_stream;

        uint16_t cache_id = 1;
        uint16_t cache_idx = 15;

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPMemBlt state_memblt(cache_id, Rect(310, 390, 10, 10), 0xFF, 0, 0, cache_idx);

        RDPOrderCommon newcommon(MEMBLT, Rect(311, 0, 800, 600));
        RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx).emit(out_stream, newcommon, state_common, state_memblt);

        uint8_t datas[9] = {
            SMALL | CHANGE | STANDARD | BOUNDS | DELTA | LASTBOUNDS,
            MEMBLT,
            0xde,   // x, y, cx, cy, srcx, srcy coordinates changed
            246,    // x = -10 -> 300 (ie: 246 is -10 using 2's complement, this avoid a c++11 warning)
            +10,    // y = 390 +10 = 400
            +40,    // cx = 10 + 40 -> 50
            +50,    // cy = 10 + 50 -> 60
              3,    // srcx = 0 + 3 = 3
              4,    // srcy = 0 + 4 = 4
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(MEMBLT), common_cmd.order);


        RDPMemBlt cmd(cache_id, Rect(310, 390, 10, 10), 0xFF, 0, 0, cache_idx);


        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_memblt);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;

        uint16_t cache_id = 1;
        uint16_t cache_idx = 15;

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPMemBlt state_memblt(0, Rect(), 0, 0, 0, 0);

        RDPOrderCommon newcommon(MEMBLT, Rect(311, 0, 800, 600));
        RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx).emit(out_stream, newcommon, state_common, state_memblt);

        uint8_t datas[21] = {
            CHANGE | STANDARD | BOUNDS | LASTBOUNDS,
            MEMBLT,
            0xff, 0x01,  // cache_id, x, y, cx, cy, rop, srcx, srcy, cache_idx changed
            0x01, 0x00,  // cache_id
            0x2C, 0x01,  // x = 300
            0x90, 0x01,  // y = 400
            0x32, 0x00,  // cx = 50
            0x3c, 0x00,  // cy = 60
            0xFF,        // rop
            0x03, 0x00,  // srcx = 3
            0x04, 0x00,  // srcy = 4
            0x0F, 0      // cache_idx
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(MEMBLT), common_cmd.order);


        RDPMemBlt cmd(0, Rect(), 0, 0, 0, 0);
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_memblt);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }
}
