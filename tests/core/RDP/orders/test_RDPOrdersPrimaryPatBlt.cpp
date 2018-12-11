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

#define RED_TEST_MODULE TestOrderPatBlt
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"


RED_AUTO_TEST_CASE(TestPatBlt)
{
    using namespace RDP;

    {
        // PATBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte
        // 0x20: Back color (3 bytes)
        // 0x40: Fore color (3 bytes)
        // 0x80: Brush Org X (1 byte)

        // 0x0100: Brush Org Y (1 byte)
        // 0x0200: Brush style (1 byte)
        // 0x0400: Brush Hatch (1 byte) + Extra if style == 0x3

        StaticOutStream<1000> out_stream;

        // DESTBLT = 0, hence we won't have order change
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPPatBlt state_patblt(Rect(), 0, RDPColor{}, RDPColor{}, RDPBrush());

        RDPOrderCommon newcommon(PATBLT, Rect(311, 0, 800, 600));
        RDPPatBlt(Rect(300, 400, 50, 60),
                  0xFF,
                  encode_color24()(BGRColor{0x102030}), encode_color24()(BGRColor{0x112233}),
                  RDPBrush(3, 4, 3, 0xDD, byte_ptr_cast("\1\2\3\4\5\6\7"))
                  ).emit(out_stream, newcommon, state_common, state_patblt);

        uint8_t datas[30] = {
            CHANGE | STANDARD | BOUNDS | LASTBOUNDS,
            PATBLT,
            0xFF, 0x0F,        // x, y, cx, cy, rop, colors and brush changed
            0x2C, 0x01,        // x = 300
            0x90, 0x01,        // y = 400
            0x32, 0x00,        // cx = 50
            0x3c, 0x00,        // cy = 60
            0xFF,              // rop
            0x30, 0x20, 0x10,  // back_color
            0x33, 0x22, 0x11,  // fore_color
            0x03,        // brush.org_x
            0x04,        // brush.org_y
            0x03,        // brush.style
            0xDD,        // brush.hatch
            1, 2, 3, 4, 5, 6, 7   // brush.extra
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(PATBLT), common_cmd.order);

        RDPPatBlt cmd(Rect(), 0, RDPColor{}, RDPColor{}, RDPBrush());

        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_patblt);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;

        // DESTBLT = 0, hence we won't have order change
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPPatBlt state_patblt(Rect(), 0, RDPColor{}, RDPColor{}, RDPBrush());

        RDPOrderCommon newcommon(PATBLT, Rect(311, 0, 800, 600));
        RDPPatBlt(Rect(300, 400, 50, 60),
                  0xFF,
                  encode_color24()(BGRColor{0x102030}), encode_color24()(BGRColor{0x112233}),
                  RDPBrush(3, 4, 1, 0xDD)
                  ).emit(out_stream, newcommon, state_common, state_patblt);

        uint8_t datas[23] = {
            CHANGE | STANDARD | BOUNDS | LASTBOUNDS,
            PATBLT,
            0xFF, 0x07,  // x, y, cx, cy, rop, colors and brush changed
            0x2C, 0x01,  // x = 300
            0x90, 0x01,  // y = 400
            0x32, 0x00,  // cx = 50
            0x3c, 0x00,  // cy = 60
            0xFF,        // rop
            0x30, 0x20, 0x10,  // back_color
            0x33, 0x22, 0x11,  // fore_color
            0x03,        // brush.org_x
            0x04,        // brush.org_y
            0x01,        // brush.style
            0xDD,        // brush.hatch
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(PATBLT), common_cmd.order);

        RDPPatBlt cmd(Rect(), 0, RDPColor{}, RDPColor{}, RDPBrush());

        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_patblt);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPPatBlt state_patblt(Rect(), 0, RDPColor{}, RDPColor{}, RDPBrush(0, 0, 0x03, 0xDD));

        RDPOrderCommon newcommon(PATBLT, Rect(311, 0, 800, 600));

        RDPPatBlt(Rect(300, 400, 50, 60),
                  0xFF,
                  encode_color24()(BGRColor{0x102030}), encode_color24()(BGRColor{0x112233}),
                  RDPBrush(3, 4, 3, 0xDD, byte_ptr_cast("\1\2\3\4\5\6\7"))
                  ).emit(out_stream, newcommon, state_common, state_patblt);

        uint8_t datas[28] = {
            CHANGE | STANDARD | BOUNDS | LASTBOUNDS,
            PATBLT,
            0xFF, 0x09,  // x, y, cx, cy, rop, bcolor, fcolor, org_x, org_y, extra changed
            0x2C, 0x01,  // x = 300
            0x90, 0x01,  // y = 400
            0x32, 0x00,  // cx = 50
            0x3c, 0x00,  // cy = 60
            0xFF,        // rop
            0x30, 0x20, 0x10,  // back_color
            0x33, 0x22, 0x11,  // fore_color
            0x03,        // brush_org_x
            0x04,        // brush_org_y
            1, 2, 3, 4, 5, 6, 7   // brush_extra
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(PATBLT), common_cmd.order);

        RDPPatBlt cmd(Rect(), 0, RDPColor{}, RDPColor{}, RDPBrush(0, 0, 3, 0xDD));

        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_patblt);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }
}
