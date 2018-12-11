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

#define RED_TEST_MODULE TestOrderLineTo
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"


RED_AUTO_TEST_CASE(TestLineTo)
{
    using namespace RDP;

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(0, Rect(0, 0, 0, 0));
        RDPLineTo state_lineto(0, 0, 0, 0, 0, RDPColor{}, 0, RDPPen(0, 0, RDPColor{}));
        RDPOrderCommon newcommon(LINE, Rect(10, 20, 30, 40));

        RDPLineTo(1, 0, 10, 40, 60, encode_color24()(BGRColor{0x102030}), 0xFF, RDPPen(0, 1, encode_color24()(BGRColor{0x112233}))
                  ).emit(out_stream, newcommon, state_common, state_lineto);


        RED_CHECK_EQUAL(static_cast<uint8_t>(LINE), newcommon.order);
        RED_CHECK_EQUAL(Rect(10, 20, 30, 40), newcommon.clip);

        uint8_t datas[] = {
            CHANGE | STANDARD | BOUNDS | DELTA,
            LINE,
            0x7D, 0x03,
            0xF0, 0x0A, 0x14, 0x28, 0x3C,
            0x01, 00, // back_mode
            0x0a, // starty = +10,
            0x28, // endx = +40
            0x3C, // endy = +60
            0x30, 0x20, 0x10, // back_color
            0xff, // rop2
            01,              // pen width
            0x33, 0x22, 0x11 // pen color
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(0x1D), header.control);
        RED_CHECK_EQUAL(static_cast<uint32_t>(0x37D), header.fields);
        RED_CHECK_EQUAL(static_cast<uint8_t>(LINE), common_cmd.order);
        RED_CHECK_EQUAL(Rect(10, 20, 30, 40), common_cmd.clip);

        RDPLineTo cmd = state_lineto;
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_lineto);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }
}
