/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    Unit test to RDP Orders coder/decoder
    Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestOrderMultiOpaqueRect
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"


RED_AUTO_TEST_CASE(TestMultiOpaqueRect)
{
    using namespace RDP;

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(0, Rect(0, 0, 0, 0));
        RDPMultiOpaqueRect state_multiopaquerect;
        state_multiopaquerect._Color = encode_color24()(BGRColor(0x00D699));
        RDPOrderCommon newcommon(MULTIOPAQUERECT, Rect(0, 0, 1024, 768));

        StaticOutStream<1024> deltaRectangles;

        deltaRectangles.out_sint16_le(316);
        deltaRectangles.out_sint16_le(378);
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);

        for (int i = 0; i < 19; i++) {
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
        }

        InStream in_deltaRectangles(deltaRectangles.get_bytes());

        RDPMultiOpaqueRect multiopaquerect(316, 378, 200, 200, encode_color24()(BLACK), 20, in_deltaRectangles);


        multiopaquerect.emit(out_stream, newcommon, state_common, state_multiopaquerect);

        RED_CHECK_EQUAL(static_cast<uint8_t>(MULTIOPAQUERECT), newcommon.order);
        RED_CHECK_EQUAL(Rect(0, 0, 0, 0), newcommon.clip);

        uint8_t datas[] = {
            CHANGE | STANDARD,
            MULTIOPAQUERECT,
            0xBF, 0x01,             // header fields
            0x3C, 0x01, 0x7a, 0x01, // nLeftRect = 316, nTopRect = 378
            0xc8, 0x00, 0xc8, 0x00, // nWidth = 200, nHeight = 200
            0x00, 0x00,             // RedOrPaletteIndex, Green
            0x14,                   // nDeltaEntries
            0x5c, 0x00,             // cbData
 /* 0000 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x3c, 0x81, 0x7a, 0x0a, 0x0a,  // ...........<.z..
 /* 0010 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0020 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0030 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0040 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0050 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,                          // ............
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(0x09), header.control);
        RED_CHECK_EQUAL(static_cast<uint32_t>(0x1BF), header.fields);
        RED_CHECK_EQUAL(static_cast<uint8_t>(MULTIOPAQUERECT), common_cmd.order);
        RED_CHECK_EQUAL(Rect(0, 0, 0, 0), common_cmd.clip);

        RDPMultiOpaqueRect cmd = state_multiopaquerect;
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_multiopaquerect);
        RED_CHECK_MEM(
            out_stream.get_bytes().array_from_offset(1),
            out_stream2.get_bytes().array_from_offset(1));
    }
}
