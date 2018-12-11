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

#define RED_TEST_MODULE TestOrderOpaqueRect
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"


RED_AUTO_TEST_CASE(TestOpaqueRect)
{
    using namespace RDP;

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(RECT, Rect(700, 200, 100, 200));
        RDPOpaqueRect state_orect(Rect(0, 0, 800, 600), RDPColor{});

        RED_CHECK_EQUAL(0, (out_stream.get_offset()));

        RDPOrderCommon newcommon(RECT, Rect(0, 400, 800, 76));
        RDPOpaqueRect(Rect(0, 0, 800, 600), RDPColor{}).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[7] = {
            SMALL | DELTA | BOUNDS | STANDARD,
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

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);
        RED_CHECK_EQUAL(0, common_cmd.clip.x);
        RED_CHECK_EQUAL(400, common_cmd.clip.y);
        RED_CHECK_EQUAL(800, common_cmd.clip.cx);
        RED_CHECK_EQUAL(76, common_cmd.clip.cy);

        RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RDPColor{});
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RED_CHECK_EQUAL(0, (out_stream.get_offset()));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[2] = {SMALL | DELTA | CHANGE | STANDARD, RECT};
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 10, 10), RDPColor::from(0xFFFFFF)).emit(out_stream, newcommon, state_common, state_orect);
        // out_stream = old - cmd

        uint8_t datas[4] = {DELTA | CHANGE | STANDARD, RECT,
            1, // x coordinate changed
            5, // +5 on x
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect newcmd(Rect(5, 10, 25, 30), RDPColor::from(0xFFFFFF));
        newcmd.emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[7] = {DELTA | CHANGE | STANDARD, RECT,
            0x0F, // x,y,w,h coordinate changed
            5,    // +5 on x
            10,   // +10 on y
            15,   // +15 on w
            20,   // +20 on h
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(0, 300, 10, 10), RDPColor::from(0xFFFFFF)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[5] = {CHANGE | STANDARD, RECT,
            2,  // y coordinate changed
            0x2C, 1 // y = 0x12C = 300
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 10, 10), RDPColor::from(0xFFFFFF)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[7] = {CHANGE | STANDARD, RECT,
               3,   // x and y coordinate changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 25, 30), RDPColor::from(0xFFFFFF)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[11] = {CHANGE | STANDARD, RECT,
            0x0F,   // x, y, w, h coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            25, 0,   // w = 25
            30, 0,   // h = 30
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 25, 30), RDPColor::from(0x102030)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[14] = {CHANGE | STANDARD, RECT,
            0x7F,   // x, y, w, h, r, g, b coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            25, 0,   // w = 25
            30, 0,   // h = 30
            0x30, 0x20, 0x10  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 300, 310, 20));
        RDPOpaqueRect(Rect(5, 300, 25, 30), RDPColor::from(0x102030)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[21] = {CHANGE | STANDARD | BOUNDS, RECT,
            0x7F,   // x, y, w, h, r, g, b coordinates changed
            0x0e,   // bounds absolutes : left, write, bottom
            0x2C, 0x01, // left bound = 300
            0x35, 0x01, // right bound = 309   (bounds are include ")
            0x3F, 0x01, // bottom bound = 319  (bounds are include ")
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            25, 0,   // w = 25
            30, 0,   // h = 30
            0x30, 0x20, 0x10  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(10, 10, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 810, 605), RDPColor::from(0x102030)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[17] = {CHANGE | STANDARD | BOUNDS, RECT,
            0x7D,   // x, w, h, r, g, b coordinates changed
            0xf0,   // bounds delta : top, left, bottom, right
            0x0A,   // left bound = +10
            0x0A,   // top bound = +10
            0x0A,   // right bound = +10
            0x0A,   // bottom bound = +10
            0x05, 0, // x = 0x005 = 5
            0x2A, 3,   // w = 810
            0x5D, 2,   // H = 605
            0x30, 0x20, 0x10,  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 810, 605), RDPColor::from(0x102030)).emit(out_stream, newcommon, state_common, state_orect);

        uint8_t datas[11] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x7D,   // x, w, h, r, g, b coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2A, 3,   // w = 810
            0x5D, 2,   // H = 605
            0x30, 0x20, 0x10,  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(RECT), common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), RDPColor::from(0xFFFFFF));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, state_common, state_orect);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

}

