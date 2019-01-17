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

#define RED_TEST_MODULE TestOrderEllipseSC
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"


RED_AUTO_TEST_CASE(TestEllipseSC)
{
    using namespace RDP;

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(ELLIPSESC, Rect(700, 200, 100, 200));
        RDPEllipseSC state_ellipse(Rect(0, 0, 800, 600), RDPColor{});

        RED_CHECK_EQUAL(0, (out_stream.get_offset()));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 400, 800, 76));
        RDPEllipseSC(Rect(0, 0, 800, 600), RDPColor{}).emit(out_stream, newcommon, state_common, state_ellipse);

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

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);
        RED_CHECK_EQUAL(0, common_cmd.clip.x);
        RED_CHECK_EQUAL(400, common_cmd.clip.y);
        RED_CHECK_EQUAL(800, common_cmd.clip.cx);
        RED_CHECK_EQUAL(76, common_cmd.clip.cy);

        RDPEllipseSC cmd(Rect(0, 0, 800, 600), RDPColor{});
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RED_CHECK_EQUAL(0, (out_stream.get_offset()));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(0, 0, 10, 10), encode_color24()(WHITE)).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[2] = {SMALL | CHANGE | STANDARD | DELTA, ELLIPSESC};
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 0, 10, 10), encode_color24()(WHITE)).emit(out_stream, newcommon, state_common, state_ellipse);
        // out_stream = old - cmd

        uint8_t datas[5] = {CHANGE | STANDARD | DELTA,
                            ELLIPSESC,
                            0x01 | 0x04, // right and left coordinate changed
                            5, // 5 on left
                            5 // 15 on right
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC newcmd(Rect(5, 10, 25, 30), encode_color24()(WHITE));
        newcmd.emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[7] = {CHANGE | STANDARD | DELTA,
                            ELLIPSESC,
                            0x0F,  // left, top, right, bottom changed
                            5, // 5 on left
                            10, // 10 on top
                            20, // 30 on right
                            30  // 40 on bottom
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(0, 300, 10, 10), encode_color24()(WHITE)).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[7] = {CHANGE | STANDARD, ELLIPSESC,
                            0x02 | 0x08, // top and bottom coordinate changed
                            0x2C, 1,     // top = 0x12C = 300
                            0x36, 1      // bottom = 0x136 = 310
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 300, 10, 10), encode_color24()(WHITE)).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[11] = {CHANGE | STANDARD, ELLIPSESC,
                             0x0f,    // left top right bottom coordinate changed
                             0x05, 0, // left = 0x005 = 5
                             0x2C, 1, // top = 0x12C = 300
                             0x0f, 0, // right = 0x0f = 15
                             0x36, 1  // 310
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 300, 25, 30), encode_color24()(WHITE)).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[11] = {CHANGE | STANDARD, ELLIPSESC,
            0x0F,   // x, y, w, h coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            30, 0,   // w = 25
            74, 1,   // h = 30
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 300, 25, 30), encode_color24()(BGRColor{0x102030})).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[14] = {CHANGE | STANDARD, ELLIPSESC,
            0x4F,   // top left right bottom coordinates and color changed
            0x05, 0, // top = 0x005 = 5
            0x2C, 1, // left = 0x12C = 300
            30, 0,   // right = 30
            74, 1,   // bottom = 330
            0x30, 0x20, 0x10  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 300, 310, 20));
        RDPEllipseSC(Rect(5, 300, 25, 30), encode_color24()(BGRColor{0x102030})).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[21] = {CHANGE | STANDARD | BOUNDS, ELLIPSESC,
            0x4F,   // x, y, w, h, r, g, b coordinates changed
            0x0e,   // bounds absolutes : left, write, bottom
            0x2C, 0x01, // left bound = 300
            0x35, 0x01, // right bound = 309   (bounds are include ")
            0x3F, 0x01, // bottom bound = 319  (bounds are include ")
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            30, 0,   // w = 25
            74, 1,   // h = 30
            0x30, 0x20, 0x10  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(10, 10, 800, 600));
        RDPEllipseSC(Rect(5, 0, 810, 605), encode_color24()(BGRColor{0x102030})).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[17] = {CHANGE | STANDARD | BOUNDS, ELLIPSESC,
            0x4D,   // x, w, h, r, g, b coordinates changed
            0xf0,   // bounds delta : top, left, bottom, right
            0x0A,   // left bound = +10
            0x0A,   // top bound = +10
            0x0A,   // right bound = +10
            0x0A,   // bottom bound = +10
            0x05, 0, // x = 0x005 = 5
            0x2F, 3,   // w = 815
            0x5D, 2,   // H = 604
            0x30, 0x20, 0x10,  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 0, 810, 605), encode_color24()(BGRColor{0x102030})).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[11] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x4D,   // x, w, h, r, g, b coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2F, 3,   // right = 815
            0x5D, 2,   // bottom = 604
            0x30, 0x20, 0x10,  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }

    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RED_CHECK_EQUAL(0, (out_stream.get_offset()));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(0, 0, 10, 10), encode_color24()(WHITE), 0x0A, 0x00).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[5] = { CHANGE | STANDARD | DELTA,
                             ELLIPSESC,
                             0x30, // brop2 and fillmode changed
                             0x0A,
                             0x00};
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        // TODO RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }


    {
        StaticOutStream<1000> out_stream;
        RDPOrderCommon state_common(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), encode_color24()(WHITE));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 0, 810, 605), encode_color24()(BGRColor{0x102030}), 0x0E, 0x00).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[13] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x7D,   // x, w, h, r, g, b coordinates rbop and fillmode changed
            0x05, 0, // x = 0x005 = 5
            0x2F, 3,   // right = 815
            0x5D, 2,   // bottom = 604
            0x0E,
            0x00,
            0x30, 0x20, 0x10,  // RGB colors
        };
        RED_CHECK_MEM(out_stream.get_bytes(), make_array_view(datas));

        InStream in_stream(out_stream.get_bytes());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSESC), common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), encode_color24()(WHITE));
        cmd.receive(in_stream, header);

        decltype(out_stream) out_stream2;
        cmd.emit(out_stream2, newcommon, common_cmd, state_ellipse);
        RED_CHECK_MEM(out_stream.get_bytes(), out_stream2.get_bytes());
    }


    RDPEllipseSC nullellipse;
    RED_CHECK(nullellipse.id() == ELLIPSESC);
    //nullellipse.log(1, Rect());
    //nullellipse.print(Rect());
}

