/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 *
 *   Unit test to RDP Orders coder/decoder
 *   Using lib boost functions for testing
 */


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAbsoluteOrderOpaqueRect
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include <algorithm>

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "transport.hpp"
#include "RDP/RDPGraphicDevice.hpp"

#include "./test_orders.hpp"

BOOST_AUTO_TEST_CASE(TestAbsoluteOpaqueRect)
{
    using namespace RDP;

    {
        BStream stream(1000);
        RDPOrderCommon state_common(RECT, Rect(700, 200, 100, 200));
        //RDPOpaqueRect state_orect(Rect(0, 0, 800, 600), 0);

        //BOOST_CHECK_EQUAL(0, (stream.get_offset()));

        Rect rect(0, 400, 800, 76);
        RDPOrderCommon newcommon(RECT, Rect(600, 500, 70,8));
        RDPOpaqueRect(rect, 0).emit(stream, newcommon/*, state_common, state_orect*/);

        uint8_t datas[14] = {
            0x09,
            RECT,
            0x7f,
            0,
            0,
            400 & 0xff,
            400 >> 8,
            800 & 0xff,
            800 >> 8,
            76 & 0xff,
            76 >> 8,
            0,
            0,
            0
        };
        check_datas(stream.get_offset(), stream.data, 14, datas, "rect draw 0");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        //BOOST_CHECK_EQUAL(0x7f, control);

        ///TODO un uint8_t utilisais en plus
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        RDPOpaqueRect cmd(Rect(0,0, 800,600), 0);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             state_common,
                             RDPOpaqueRect(rect, 0),
                             "rect draw 0");
    }


    /*

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        BOOST_CHECK_EQUAL(0, (stream.get_offset()));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(0, 0, 10, 10), 0xFFFFFF).emit(stream, newcommon);

        uint8_t datas[2] = {SMALL | DELTA | CHANGE | STANDARD, RECT};
        check_datas(stream.get_offset(), stream.data, 2, datas, "rect draw identical");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(0, 0, 10, 10), 0xFFFFFF),
                             "rect draw identical");
    }


    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 10, 10), 0xFFFFFF).emit(stream, newcommon);
        // stream = old - cmd

        uint8_t datas[4] = {DELTA | CHANGE | STANDARD, RECT,
        1, // x coordinate changed
        5, // +5 on x
        };
        check_datas(stream.get_offset(), stream.data, 4, datas, "rect draw 1");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(5, 0, 10, 10), 0xFFFFFF),
                             "rect draw 1");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect newcmd(Rect(5, 10, 25, 30), 0xFFFFFF);
        newcmd.emit(stream, newcommon);

        uint8_t datas[7] = {DELTA | CHANGE | STANDARD, RECT,
        0x0F, // x,y,w,h coordinate changed
        5,    // +5 on x
        10,   // +10 on y
        15,   // +15 on w
        20,   // +20 on h
        };
        check_datas(stream.p-stream.data, stream.data, 7, datas, "rect draw 2");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(5, 10, 25, 30), 0xFFFFFF),
                             "rect draw 2");
    }


    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(0, 300, 10, 10), 0xFFFFFF).emit(stream, newcommon);

        uint8_t datas[5] = {CHANGE | STANDARD, RECT,
        2,  // y coordinate changed
        0x2C, 1 // y = 0x12C = 300
        };
        check_datas(stream.p-stream.data, stream.data, 5, datas, "rect draw 3");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(0, 300, 10, 10), 0xFFFFFF),
                             "rect draw 3");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 10, 10), 0xFFFFFF).emit(stream, newcommon);

        uint8_t datas[7] = {CHANGE | STANDARD, RECT,
        3,   // x and y coordinate changed
        0x05, 0, // x = 0x005 = 5
        0x2C, 1, // y = 0x12C = 300
        };
        check_datas(stream.p-stream.data, stream.data, 7, datas, "rect draw 4");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(5, 300, 10, 10), 0xFFFFFF),
                             "rect draw 4");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 25, 30), 0xFFFFFF).emit(stream, newcommon);

        uint8_t datas[11] = {CHANGE | STANDARD, RECT,
        0x0F,   // x, y, w, h coordinates changed
        0x05, 0, // x = 0x005 = 5
        0x2C, 1, // y = 0x12C = 300
        25, 0,   // w = 25
        30, 0,   // h = 30
        };
        check_datas(stream.p-stream.data, stream.data, 11, datas, "rect draw 5");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(5, 300, 25, 30), 0xFFFFFF),
                             "rect draw 5");

    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 25, 30), 0x102030).emit(stream, newcommon);

        uint8_t datas[14] = {CHANGE | STANDARD, RECT,
        0x7F,   // x, y, w, h, r, g, b coordinates changed
        0x05, 0, // x = 0x005 = 5
        0x2C, 1, // y = 0x12C = 300
        25, 0,   // w = 25
        30, 0,   // h = 30
        0x30, 0x20, 0x10  // RGB colors
        };
        check_datas(stream.p-stream.data, stream.data, 14, datas, "rect draw 6");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(5, 300, 25, 30), 0x102030),
                             "rect draw 6");
    }

    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 300, 310, 20));
        RDPOpaqueRect(Rect(5, 300, 25, 30), 0x102030).emit(stream, newcommon);

        uint8_t datas[21] = {CHANGE | STANDARD | BOUNDS, RECT,
        0x7F,   // x, y, w, h, r, g, b coordinates changed
        0x0e,   // bounds absolutes : left, write, bottom
        0x2C, 0x01, // left bound = 300
        0x35, 0x01, // right bound = 309   (bounds are included)
        0x3F, 0x01, // bottom bound = 319  (bounds are included)
        0x05, 0, // x = 0x005 = 5
        0x2C, 1, // y = 0x12C = 300
        25, 0,   // w = 25
        30, 0,   // h = 30
        0x30, 0x20, 0x10  // RGB colors
        };
        check_datas(stream.p-stream.data, stream.data, 21, datas, "rect draw 7");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 300, 310, 20)),
                             RDPOpaqueRect(Rect(5, 300, 25, 30), 0x102030),
                             "rect draw 7");
    }


    {
        BStream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(10, 10, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 810, 605), 0x102030).emit(stream, newcommon);

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
        check_datas(stream.p-stream.data, stream.data, 17, datas, "rect draw 8");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(10, 10, 800, 600)),
                             RDPOpaqueRect(Rect(5, 0, 810, 605), 0x102030),
                             "rect draw 8");
    }


    {
        BStream stream(1000);
        RDPOrderCommon state_common(RECT, Rect(0, 0, 800, 600));
        //RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 810, 605), 0x102030).emit(stream, newcommon);

        uint8_t datas[11] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x7D,   // x, w, h, r, g, b coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2A, 3,   // w = 810
            0x5D, 2,   // H = 605
            0x30, 0x20, 0x10,  // RGB colors
        };
        check_datas(stream.p-stream.data, stream.data, 11, datas, "Rect Draw 9");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);

        RDPOpaqueRect cmd(Rect(0, 0, 10, 10), 0xFFFFFF);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
                             RDPOrderCommon(RECT, Rect(0, 0, 800, 600)),
                             RDPOpaqueRect(Rect(5, 0, 810, 605), 0x102030),
                             "Rect Draw 9");

    }*/

}

