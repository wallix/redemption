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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOrderOpaqueRect
#include <boost/test/auto_unit_test.hpp>
#include <algorithm>

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"

#include "test_orders.hpp"

BOOST_AUTO_TEST_CASE(TestOpaqueRect)
{
    using namespace RDP;

    {
        BStream stream(1000);
        RDPOrderCommon state_common(ELLIPSESC, Rect(700, 200, 100, 200));
        RDPEllipseSC state_ellipse(Rect(0, 0, 800, 600), 0);

        BOOST_CHECK_EQUAL(0, (stream.get_offset()));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 400, 800, 76));
        RDPEllipseSC(Rect(0, 0, 800, 600), 0).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[7] = {
            SMALL | BOUNDS | STANDARD,
            0x83,
            0x00,
            0x00,
            0x90,
            0x01,
            0x4C };
        check_datas(stream.get_offset(), stream.get_data(), 7, datas, "ellipsesc draw 01");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);
        BOOST_CHECK_EQUAL(0, common_cmd.clip.x);
        BOOST_CHECK_EQUAL(400, common_cmd.clip.y);
        BOOST_CHECK_EQUAL(800, common_cmd.clip.cx);
        BOOST_CHECK_EQUAL(76, common_cmd.clip.cy);

        RDPEllipseSC cmd(Rect(0, 0, 800, 600), 0);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 400, 800, 76)),
            RDPEllipseSC(Rect(0, 0, 800, 600), 0),
                            "ellipsesc draw 0");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        BOOST_CHECK_EQUAL(0, (stream.get_offset()));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(0, 0, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[2] = {SMALL | CHANGE | STANDARD, ELLIPSESC};
        check_datas(stream.get_offset(), stream.get_data(), 2, datas, "ellipse draw identical");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(0, 0, 10, 10), 0xFFFFFF),
            "ellipse draw identical");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 0, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_ellipse);
        // stream = old - cmd

        uint8_t datas[7] = {CHANGE | STANDARD,
                            ELLIPSESC,
                            0x01 | 0x04, // right and left coordinate changed
                            5, 0, // 5 on left
                            15, 0 // 15 on right
        };
        check_datas(stream.get_offset(), stream.get_data(), 7, datas, "ellipse draw 1");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(5, 0, 10, 10), 0xFFFFFF),
            "ellipse draw 1");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC newcmd(Rect(5, 10, 25, 30), 0xFFFFFF);
        newcmd.emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[11] = {CHANGE | STANDARD, ELLIPSESC,
                            0x0F,  // left, top, right, bottom changed
                            5,  0, // 5 on left
                            10, 0, // 10 on top
                            29, 0, // 29 on right
                            40, 0  // 40 on bottom
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 11, datas, "ellipse draw 2");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(5, 10, 25, 30), 0xFFFFFF),
            "ellipse draw 2");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(0, 300, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[7] = {CHANGE | STANDARD, ELLIPSESC,
                            0x02 | 0x08, // top and bottom coordinate changed
                            0x2C, 1,     // top = 0x12C = 300
                            0x36, 1      // bottom = 0x136 = 310
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 7, datas, "ellipse draw 3");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(0, 300, 10, 10), 0xFFFFFF),
                            "ellipse draw 3");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 300, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[11] = {CHANGE | STANDARD, ELLIPSESC,
                             0x0f,    // left top right bottom coordinate changed
                             0x05, 0, // left = 0x005 = 5
                             0x2C, 1, // top = 0x12C = 300
                             0x0f, 0, // right = 0x0f = 15
                             0x36, 1  // 310
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 11, datas, "ellipse draw 4");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
                            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
                            RDPEllipseSC(Rect(5, 300, 10, 10), 0xFFFFFF),
                            "ellipse draw 4");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 300, 25, 30), 0xFFFFFF).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[11] = {CHANGE | STANDARD, ELLIPSESC,
            0x0F,   // x, y, w, h coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            29, 0,   // w = 25
            74, 1,   // h = 30
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 11, datas, "ellipse draw 5");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(5, 300, 25, 30), 0xFFFFFF),
            "ellipse draw 5");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 300, 25, 30), 0x102030).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[14] = {CHANGE | STANDARD, ELLIPSESC,
            0x4F,   // top left right bottom coordinates and color changed
            0x05, 0, // top = 0x005 = 5
            0x2C, 1, // left = 0x12C = 300
            29, 0,   // right = 29
            74, 1,   // bottom = 330
            0x30, 0x20, 0x10  // RGB colors
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 14, datas, "ellipse draw 6");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(5, 300, 25, 30), 0x102030),
            "ellipse draw 6");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 300, 310, 20));
        RDPEllipseSC(Rect(5, 300, 25, 30), 0x102030).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[21] = {CHANGE | STANDARD | BOUNDS, ELLIPSESC,
            0x4F,   // x, y, w, h, r, g, b coordinates changed
            0x0e,   // bounds absolutes : left, write, bottom
            0x2C, 0x01, // left bound = 300
            0x35, 0x01, // right bound = 309   (bounds are include ")
            0x3F, 0x01, // bottom bound = 319  (bounds are include ")
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            29, 0,   // w = 25
            74, 1,   // h = 30
            0x30, 0x20, 0x10  // RGB colors
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 21, datas, "ellipse draw 7");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 300, 310, 20)),
            RDPEllipseSC(Rect(5, 300, 25, 30), 0x102030),
            "ellipse draw 7");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(10, 10, 800, 600));
        RDPEllipseSC(Rect(5, 0, 810, 605), 0x102030).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[17] = {CHANGE | STANDARD | BOUNDS, ELLIPSESC,
            0x4D,   // x, w, h, r, g, b coordinates changed
            0xf0,   // bounds delta : top, left, bottom, right
            0x0A,   // left bound = +10
            0x0A,   // top bound = +10
            0x0A,   // right bound = +10
            0x0A,   // bottom bound = +10
            0x05, 0, // x = 0x005 = 5
            0x2F, 3,   // w = 815
            0x5C, 2,   // H = 604
            0x30, 0x20, 0x10,  // RGB colors
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 17, datas, "ellipse draw 8");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(10, 10, 800, 600)),
            RDPEllipseSC(Rect(5, 0, 810, 605), 0x102030),
            "ellipse draw 8");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 0, 810, 605), 0x102030).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[11] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x4D,   // x, w, h, r, g, b coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2F, 3,   // right = 815
            0x5C, 2,   // bottom = 604
            0x30, 0x20, 0x10,  // RGB colors
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 11, datas, "Ellipse Draw 9");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(5, 0, 810, 605), 0x102030),
            "Ellipse Draw 9");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        BOOST_CHECK_EQUAL(0, (stream.get_offset()));

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(0, 0, 10, 10), 0xFFFFFF, 0x0A, 0x00).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[5] = { CHANGE | STANDARD, ELLIPSESC,
                             0x30, // brop2 and fillmode changed
                             0x0A,
                             0x00};
        check_datas(stream.get_offset(), stream.get_data(), 5, datas, "ellipse draw 10");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(0, 0, 10, 10), 0xFFFFFF, 0x0A, 0x00),
            "ellipse draw 10");
    }


    {
        BStream stream(1000);
        RDPOrderCommon state_common(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC state_ellipse(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(ELLIPSESC, Rect(0, 0, 800, 600));
        RDPEllipseSC(Rect(5, 0, 810, 605), 0x102030, 0x0E, 0x00).emit(stream, newcommon, state_common, state_ellipse);

        uint8_t datas[13] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x7D,   // x, w, h, r, g, b coordinates rbop and fillmode changed
            0x05, 0, // x = 0x005 = 5
            0x2F, 3,   // right = 815
            0x5C, 2,   // bottom = 604
            0x0E,
            0x00,
            0x30, 0x20, 0x10,  // RGB colors
        };
        check_datas(stream.p-stream.get_data(), stream.get_data(), 13, datas, "Ellipse Draw 11");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)ELLIPSESC, common_cmd.order);

        RDPEllipseSC cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPEllipseSC>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSESC, Rect(0, 0, 800, 600)),
            RDPEllipseSC(Rect(5, 0, 810, 605), 0x102030, 0x0E, 0x00),
            "Ellipse Draw 11");
    }


    RDPEllipseSC nullellipse;
    BOOST_CHECK(nullellipse.id() == ELLIPSESC);
    nullellipse.log(1, Rect());
    nullellipse.print(Rect());
}

