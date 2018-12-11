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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
  Meng Tan

  Unit test to RDP Orders coder/decoder
  Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestOrderPolygonSC
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"


RED_AUTO_TEST_CASE(TestPolygonSCEmpty)
{
    using namespace RDP;
    StaticOutStream<1000> out_stream;
    RDPOrderCommon state_common(POLYGONSC, Rect(700, 200, 100, 200));
    RDPPolygonSC state_Polygon;

    RED_CHECK_EQUAL(0, (out_stream.get_offset()));

    RDPOrderCommon newcommon(POLYGONSC, Rect(0, 400, 800, 76));
    RDPPolygonSC().emit(out_stream, newcommon, state_common, state_Polygon);

    uint8_t datas[7] = {
        SMALL | BOUNDS | STANDARD | DELTA,
        0x83,
        0x00,
        0x00,
        0x90,
        0x01,
        0x4C };
    RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

    InStream in_stream(out_stream.get_bytes());

    RDPOrderCommon common_cmd = state_common;
    uint8_t control = in_stream.in_uint8();
    RED_CHECK_EQUAL(true, !!(control & STANDARD));
    RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

    RED_CHECK_EQUAL(static_cast<uint8_t>(POLYGONSC), common_cmd.order);
    RED_CHECK_EQUAL(0, common_cmd.clip.x);
    RED_CHECK_EQUAL(400, common_cmd.clip.y);
    RED_CHECK_EQUAL(800, common_cmd.clip.cx);
    RED_CHECK_EQUAL(76, common_cmd.clip.cy);

    RDPPolygonSC cmd;
    cmd.receive(in_stream, header);

    decltype(out_stream) out_stream2;
    cmd.emit(out_stream2, newcommon, state_common, state_Polygon);
    RED_CHECK_MEM(
        out_stream.get_bytes().array_from_offset(1),
        out_stream2.get_bytes().array_from_offset(1));
}
RED_AUTO_TEST_CASE(TestPolygonSC)
{
    using namespace RDP;
    StaticOutStream<1000> out_stream;

    RDPOrderCommon state_common(0, Rect(0, 0, 0, 0));
    RDPPolygonSC state_polygonSC;
    RDPOrderCommon newcommon(POLYGONSC, Rect(0, 0, 1024, 768));

    StaticOutStream<1024> deltaPoints;

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(20);

    deltaPoints.out_sint16_le(160);
    deltaPoints.out_sint16_le(0);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(-30);

    deltaPoints.out_sint16_le(50);
    deltaPoints.out_sint16_le(50);

    deltaPoints.out_sint16_le(-50);
    deltaPoints.out_sint16_le(50);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(-30);

    deltaPoints.out_sint16_le(-160);
    deltaPoints.out_sint16_le(0);

    InStream deltaPoints_in(deltaPoints.get_bytes());

    RDPPolygonSC polygonSC(158, 230, 0x0D, 0, encode_color24()(BLACK), 7, deltaPoints_in);


    polygonSC.emit(out_stream, newcommon, state_common, state_polygonSC);

    RED_CHECK_EQUAL(static_cast<uint8_t>(POLYGONSC), newcommon.order);
    RED_CHECK_EQUAL(Rect(0, 0, 0, 0), newcommon.clip);

    uint8_t datas[] = {
        CHANGE | STANDARD,
        POLYGONSC,
        0x67,
        0x9E, 0x00, 0xE6, 0x00, // xStart = 158, yStart = 203
        0x0D,                   // bRop2
        0x07,                   // NumDeltaEntries
        0x0D,                   // cbData
        0x98, 0x24, 0x14, 0x80, 0xA0, 0x62, 0x32, 0x32,
        0x4E, 0x32, 0x62, 0xFF, 0x60
    };
    RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

    InStream in_stream(out_stream.get_bytes());

    RDPOrderCommon common_cmd = state_common;
    uint8_t control = in_stream.in_uint8();
    RED_CHECK_EQUAL(true, !!(control & STANDARD));
    RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

    RED_CHECK_EQUAL(static_cast<uint8_t>(0x09), header.control);
    RED_CHECK_EQUAL(static_cast<uint32_t>(0x67), header.fields);
    RED_CHECK_EQUAL(static_cast<uint8_t>(POLYGONSC), common_cmd.order);
    RED_CHECK_EQUAL(Rect(0, 0, 0, 0), common_cmd.clip);

    RDPPolygonSC cmd = state_polygonSC;
    cmd.receive(in_stream, header);

    decltype(out_stream) out_stream2;
    cmd.emit(out_stream2, newcommon, state_common, state_polygonSC);
    RED_CHECK_MEM(
        out_stream.get_bytes().array_from_offset(1),
        out_stream2.get_bytes().array_from_offset(1));
}
