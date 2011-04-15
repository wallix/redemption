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
#define BOOST_TEST_MODULE TestOrdersState
#include <boost/test/auto_unit_test.hpp>
#include <algorithm>

#include "NewRDPOrders.hpp"

void check_datas(size_t lg_data, uint8_t * data,
                 size_t lg_result, uint8_t * expected_result,
                 const char * message)
{
    size_t lg_min = std::min(lg_result,lg_data);
    if (lg_result != lg_data){
        printf("Got      %u:", (unsigned)lg_data);
        size_t j = 0;
        for (j = 0; j < lg_data; j++){
            printf(" %.2x,", data[j]);
        }
        printf("\n");
        printf("Expected %u:", (unsigned)lg_result);
        for (j = 0; j < lg_result; j++){
            printf(" %.2x,", expected_result[j]);
        }
        printf("\n");
        size_t i = 0;
        for (i = 0; i < lg_min; i++){
            if (data[i] != expected_result[i]){
                break;
            }
        }
        BOOST_CHECK_MESSAGE(lg_result == lg_data,
            "test " << message << ": length mismatch\n"
            "Expected " << lg_result << "\n"
            "Got " << lg_data << "\n"
            "Data differs at index " << i << "\n"
            "Expected " << (int)expected_result[i] << "\n"
            "Got " << (int)data[i] << "\n");
        return;
    }

    for (size_t i = 0; i < lg_result; i++){
        if (expected_result[i] != data[i]){
            printf("Got      %u:", (unsigned)lg_data);
            for (size_t j = 0; j < lg_data; j++){
                printf(" %.2x,", data[j]);
            }
            printf("\n");
            printf("Expected %u:", (unsigned)lg_result);
            for (size_t j = 0; j < lg_result; j++){
                printf(" %.2x,", expected_result[j]);
            }
            printf("\n");
        }
        BOOST_CHECK_MESSAGE(expected_result[i] == data[i],
            "test "         << message << " :"
            << " expected " << (int)expected_result[i]
            << " got "      << (int)data[i]
            << " at index " << i
            << "\n");
    }
}

template <class RDPOrderType>
void check(const RDPOrderCommon & common,
           const RDPOrderType & cmd,
           const RDPOrderCommon & expected_common,
           const RDPOrderType & expected_cmd,
           const char * message)
{
    char buffer[1024];
    size_t idx = 0;
    idx += snprintf(buffer + idx,   1024 - idx, "%s:\n", message);
    idx += snprintf(buffer + idx,   1024 - idx, "Expected ");
    idx += expected_cmd.str(buffer + idx, 1024 - idx, expected_common);
    idx += snprintf(buffer + idx,   1024 - idx, "Got ");
    idx += cmd.str(buffer + idx, 1024 - idx, common);
    buffer[1023] = 0;

    BOOST_CHECK_MESSAGE((expected_common == common) && (expected_cmd == cmd), buffer);
}

template <class RDPOrderType>
void check(const RDPOrderType & cmd, const RDPOrderType & expected_cmd,
           const char * message)
{
    char buffer[2048];
    size_t idx = 0;
    idx += snprintf(buffer + idx,   1024 - idx, "%s:\n", message);
    idx += snprintf(buffer + idx,   1024 - idx, "Expected ");
    idx += expected_cmd.str(buffer + idx, 1024 - idx);
    idx += snprintf(buffer + idx,   1024 - idx, "Got ");
    idx += cmd.str(buffer + idx, 1024 - idx);
    buffer[1023] = 0;

    BOOST_CHECK_MESSAGE(expected_cmd == cmd, buffer);
}

BOOST_AUTO_TEST_CASE(TestOrdersState)
{
    using namespace RDP;

    {
        Stream stream(1000);
        RDPOrderCommon state_common(RECT, Rect(700, 200, 100, 200));
        RDPOpaqueRect state_orect(Rect(0, 0, 800, 600), 0);

        BOOST_CHECK_EQUAL(0, (stream.p - stream.data));

        RDPOrderCommon newcommon(RECT, Rect(0, 400, 800, 76));
        RDPOpaqueRect(Rect(0, 0, 800, 600), 0).emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[7] = {
            SMALL | DELTA | BOUNDS | STANDARD,
            0x83,
            0x00,
            0x00,
            0x90,
            0x01,
            0x4C };
        check_datas(stream.p - stream.data, stream.data, 7, datas, "rect draw 0");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)RECT, common_cmd.order);
        BOOST_CHECK_EQUAL(0, common_cmd.clip.x);
        BOOST_CHECK_EQUAL(400, common_cmd.clip.y);
        BOOST_CHECK_EQUAL(800, common_cmd.clip.cx);
        BOOST_CHECK_EQUAL(76, common_cmd.clip.cy);

        RDPOpaqueRect cmd(Rect(0, 0, 800, 600), 0);
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common_cmd, cmd,
            RDPOrderCommon(RECT, Rect(0, 400, 800, 76)),
            RDPOpaqueRect(Rect(0, 0, 800, 600), 0),
            "rect draw 0");
    }

    {
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        BOOST_CHECK_EQUAL(0, (stream.p - stream.data));

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(0, 0, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[2] = {SMALL | DELTA | CHANGE | STANDARD, RECT};
        check_datas(stream.p - stream.data, stream.data, 2, datas, "rect draw identical");

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderState state;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_orect);
        // stream = old - cmd

        uint8_t datas[4] = {DELTA | CHANGE | STANDARD, RECT,
            1, // x coordinate changed
            5, // +5 on x
        };
        check_datas(stream.p - stream.data, stream.data, 4, datas, "rect draw 1");

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect newcmd(Rect(5, 10, 25, 30), 0xFFFFFF);
        newcmd.emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[7] = {DELTA | CHANGE | STANDARD, RECT,
            0x0F, // x,y,w,h coordinate changed
            5,    // +5 on x
            10,   // +10 on y
            15,   // +15 on w
            20,   // +20 on h
        };
        check_datas(stream.p-stream.data, stream.data, 7, datas, "rect draw 2");

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderState state;
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(0, 300, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[5] = {CHANGE | STANDARD, RECT,
            2,  // y coordinate changed
            0x2C, 1 // y = 0x12C = 300
        };
        check_datas(stream.p-stream.data, stream.data, 5, datas, "rect draw 3");

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 10, 10), 0xFFFFFF).emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[7] = {CHANGE | STANDARD, RECT,
               3,   // x and y coordinate changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
        };
        check_datas(stream.p-stream.data, stream.data, 7, datas, "rect draw 4");

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 25, 30), 0xFFFFFF).emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[11] = {CHANGE | STANDARD, RECT,
            0x0F,   // x, y, w, h coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            25, 0,   // w = 25
            30, 0,   // h = 30
        };
        check_datas(stream.p-stream.data, stream.data, 11, datas, "rect draw 5");

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 300, 25, 30), 0x102030).emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[14] = {CHANGE | STANDARD, RECT,
            0x7F,   // x, y, w, h, r, g, b coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2C, 1, // y = 0x12C = 300
            25, 0,   // w = 25
            30, 0,   // h = 30
            0x30, 0x20, 0x10  // RGB colors
        };
        check_datas(stream.p-stream.data, stream.data, 14, datas, "rect draw 6");

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 300, 310, 20));
        RDPOpaqueRect(Rect(5, 300, 25, 30), 0x102030).emit(stream, newcommon, state_common, state_orect);

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

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(10, 10, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 810, 605), 0x102030).emit(stream, newcommon, state_common, state_orect);

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

        stream.end = stream.p; stream.p = stream.data;

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
        Stream stream(1000);
        RDPOrderCommon state_common(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect state_orect(Rect(0, 0, 10, 10), 0xFFFFFF);

        RDPOrderCommon newcommon(RECT, Rect(0, 0, 800, 600));
        RDPOpaqueRect(Rect(5, 0, 810, 605), 0x102030).emit(stream, newcommon, state_common, state_orect);

        uint8_t datas[11] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x7D,   // x, w, h, r, g, b coordinates changed
            0x05, 0, // x = 0x005 = 5
            0x2A, 3,   // w = 810
            0x5D, 2,   // H = 605
            0x30, 0x20, 0x10,  // RGB colors
        };
        check_datas(stream.p-stream.data, stream.data, 11, datas, "Rect Draw 9");

        stream.end = stream.p; stream.p = stream.data;

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

    }

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
        RDPOrderState state;
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

    {
        Stream stream(1000);

        uint16_t cache_id = 1;
        uint16_t cache_idx = 15;

        RDPOrderState state;
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPMemBlt state_memblt(cache_id, Rect(310, 390, 10, 10), 0xFF, 0, 0, cache_idx);

        RDPOrderCommon newcommon(MEMBLT, Rect(311, 0, 800, 600));
        RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx).emit(stream, newcommon, state_common, state_memblt);

        uint8_t datas[9] = {
            SMALL | CHANGE | STANDARD | BOUNDS | DELTA | LASTBOUNDS,
            MEMBLT,
            0xde,   // x, y, cx, cy, srcx, srcy coordinates changed
            -10,    // x = -10 -> 300
            +10,    // y = 390 +10 = 400
            +40,    // cx = 10 + 40 -> 50
            +50,    // cy = 10 + 50 -> 60
              3,    // srcx = 0 + 3 = 3
              4,    // srcy = 0 + 4 = 4
        };
        check_datas(stream.p-stream.data, stream.data, 9, datas, "MemBlt 1");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)MEMBLT, common_cmd.order);


        RDPMemBlt cmd(cache_id, Rect(310, 390, 10, 10), 0xFF, 0, 0, cache_idx);


        cmd.receive(stream, header);

        check<RDPMemBlt>(common_cmd, cmd,
            RDPOrderCommon(MEMBLT, Rect(311, 0, 800, 600)),
            RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx),
            "MemBlt 1");
    }

    {
        Stream stream(1000);

        uint16_t cache_id = 1;
        uint16_t cache_idx = 15;

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPMemBlt state_memblt(0, Rect(), 0, 0, 0, 0);

        RDPOrderCommon newcommon(MEMBLT, Rect(311, 0, 800, 600));
        RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx).emit(stream, newcommon, state_common, state_memblt);

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
        check_datas(stream.p-stream.data, stream.data, 21, datas, "MemBlt 2");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)MEMBLT, common_cmd.order);


        RDPMemBlt cmd(0, Rect(), 0, 0, 0, 0);


        cmd.receive(stream, header);

        check<RDPMemBlt>(common_cmd, cmd,
            RDPOrderCommon(MEMBLT, Rect(311, 0, 800, 600)),
            RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx),
            "MemBlt 2");
    }

    {
        Stream stream(1000);

        // DESTBLT = 0, hence we won't have order change
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPDestBlt state_destblt(Rect(), 0);

        RDPOrderCommon newcommon(DESTBLT, Rect(311, 0, 800, 600));
        RDPDestBlt(Rect(300, 400, 50, 60), 0xFF).emit(stream, newcommon, state_common, state_destblt);

        uint8_t datas[11] = {
            STANDARD | BOUNDS | LASTBOUNDS,
            0x1F,        // x, y, cx, cy, rop changed
            0x2C, 0x01,  // x = 300
            0x90, 0x01,  // y = 400
            0x32, 0x00,  // cx = 50
            0x3c, 0x00,  // cy = 60
            0xFF,        // rop
        };
        check_datas(stream.p-stream.data, stream.data, 11, datas, "DestBlt 1");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)DESTBLT, common_cmd.order);

        RDPDestBlt cmd(Rect(), 0);

        cmd.receive(stream, header);

        check<RDPDestBlt>(common_cmd, cmd,
            RDPOrderCommon(DESTBLT, Rect(311, 0, 800, 600)),
            RDPDestBlt(Rect(300, 400, 50, 60), 0xFF),
            "DestBlt 1");
    }

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

        Stream stream(1000);

        // DESTBLT = 0, hence we won't have order change
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPPatBlt state_patblt(Rect(), 0, 0, 0, RDPBrush());

        RDPOrderCommon newcommon(PATBLT, Rect(311, 0, 800, 600));
        RDPPatBlt(Rect(300, 400, 50, 60),
                  0xFF,
                  0x102030, 0x112233,
                  RDPBrush(3, 4, 3, 0xDD, (uint8_t*)"\1\2\3\4\5\6\7")
                  ).emit(stream, newcommon, state_common, state_patblt);

        uint8_t datas[30] = {
            CHANGE | STANDARD | BOUNDS | LASTBOUNDS,
            PATBLT,
            0xFF, 0x0F,  // x, y, cx, cy, rop, colors and brush changed
            0x2C, 0x01,  // x = 300
            0x90, 0x01,  // y = 400
            0x32, 0x00,  // cx = 50
            0x3c, 0x00,  // cy = 60
            0xFF,        // rop
            0x30, 0x20, 0x10,  // back_color
            0x33, 0x22, 0x11,  // fore_color
            0x03,        // brush.org_x
            0x04,        // brush.org_y
            0x03,        // brush.style
            0xDD,        // brush.hatch
            1, 2, 3, 4, 5, 6, 7   // brush.extra
        };
        check_datas(stream.p-stream.data, stream.data, 30, datas, "PatBlt 1");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)PATBLT, common_cmd.order);

        RDPPatBlt cmd(Rect(), 0, 0, 0, RDPBrush());

        cmd.receive(stream, header);

        check<RDPPatBlt>(common_cmd, cmd,
            RDPOrderCommon(PATBLT, Rect(311, 0, 800, 600)),
            RDPPatBlt(Rect(300, 400, 50, 60), 0xFF, 0x102030, 0x112233,
                RDPBrush(3, 4, 0x03, 0xDD, (uint8_t*)"\1\2\3\4\5\6\7")),
            "PatBlt 1");
    }

    {
        Stream stream(1000);

        // DESTBLT = 0, hence we won't have order change
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPPatBlt state_patblt(Rect(), 0, 0, 0, RDPBrush());

        RDPOrderCommon newcommon(PATBLT, Rect(311, 0, 800, 600));
        RDPPatBlt(Rect(300, 400, 50, 60),
                  0xFF,
                  0x102030, 0x112233,
                  RDPBrush(3, 4, 1, 0xDD)
                  ).emit(stream, newcommon, state_common, state_patblt);

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
        check_datas(stream.p-stream.data, stream.data, 23, datas, "PatBlt 2");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)PATBLT, common_cmd.order);

        RDPPatBlt cmd(Rect(), 0, 0, 0, RDPBrush());

        cmd.receive(stream, header);

        check<RDPPatBlt>(common_cmd, cmd,
            RDPOrderCommon(PATBLT, Rect(311, 0, 800, 600)),
            RDPPatBlt(Rect(300, 400, 50, 60), 0xFF, 0x102030, 0x112233,
                RDPBrush(3, 4, 0x01, 0xDD)),
            "PatBlt 2");
    }

    {
        Stream stream(1000);

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPPatBlt state_patblt(Rect(), 0, 0, 0, RDPBrush(0, 0, 0x03, 0xDD));

        RDPOrderCommon newcommon(PATBLT, Rect(311, 0, 800, 600));

        RDPPatBlt(Rect(300, 400, 50, 60),
                  0xFF,
                  0x102030, 0x112233,
                  RDPBrush(3, 4, 3, 0xDD, (uint8_t*)"\1\2\3\4\5\6\7")
                  ).emit(stream, newcommon, state_common, state_patblt);

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
        check_datas(stream.p-stream.data, stream.data, 28, datas, "PatBlt 3");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)PATBLT, common_cmd.order);

        RDPPatBlt cmd(Rect(), 0, 0, 0, RDPBrush(0, 0, 3, 0xDD));

        cmd.receive(stream, header);

        check<RDPPatBlt>(common_cmd, cmd,
            RDPOrderCommon(PATBLT, Rect(311, 0, 800, 600)),
            RDPPatBlt(Rect(300, 400, 50, 60), 0xFF, 0x102030, 0x112233,
                RDPBrush(3, 4, 0x03, 0xDD, (uint8_t*)"\1\2\3\4\5\6\7")),
            "PatBlt 3");
    }

    {
        Stream stream(1000);

        RDPOrderCommon state_common(0, Rect(0, 0, 0, 0));
        RDPLineTo state_lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        RDPOrderCommon newcommon(LINE, Rect(10, 20, 30, 40));

        RDPLineTo(1, 0, 10, 40, 60, 0x102030, 0xFF, RDPPen(0, 1, 0x112233)
                  ).emit(stream, newcommon, state_common, state_lineto);


        BOOST_CHECK_EQUAL((uint8_t)LINE, newcommon.order);
        BOOST_CHECK_EQUAL(Rect(10, 20, 30, 40), newcommon.clip);

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
        check_datas(stream.p-stream.data, stream.data, sizeof(datas), datas, "LineTo 1");

        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)0x1D, header.control);
        BOOST_CHECK_EQUAL((uint32_t)0x37D, header.fields);
        BOOST_CHECK_EQUAL((uint8_t)LINE, common_cmd.order);
        BOOST_CHECK_EQUAL(Rect(10, 20, 30, 40), common_cmd.clip);

        RDPLineTo cmd = state_lineto;
        cmd.receive(stream, header);

        check<RDPLineTo>(common_cmd, cmd,
            RDPOrderCommon(LINE, Rect(10, 20, 30, 40)),
            RDPLineTo(1, 0, 10, 40, 60, 0x102030, 0xFF, RDPPen(0, 1, 0x112233)),
            "LineTo 1");
    }

    {
        Stream stream(1000);

        RDPOrderCommon state_common(0, Rect(0, 0, 0, 0));
        RDPGlyphIndex statecmd(0, 0, 0, 0, 0, 0, Rect(), Rect(), RDPBrush(), 0, 0, 0, (uint8_t*)"");
        RDPOrderCommon newcommon(GLYPHINDEX, Rect(5, 0, 800, 600));
        RDPGlyphIndex newcmd(1, 0x20, 1, 4,
                             0x112233,
                             0x445566,
                             Rect(1,2,40,60),
                             Rect(3,4,50,70),
                             RDPBrush(3, 4, 0x03, 0xDD, (uint8_t*)"\1\2\3\4\5\6\7"),
                             5, 6,
     #warning actual data is much more complex, than a text, we should create a spécialized object to store, serialize and replay it. This should be done after the RDP layer includes cache management primitives
                             12, (uint8_t*)"Hello, World");

        newcmd.emit(stream, newcommon, state_common, statecmd);

        BOOST_CHECK_EQUAL((uint8_t)GLYPHINDEX, newcommon.order);
        BOOST_CHECK_EQUAL(Rect(5, 0, 800, 600), newcommon.clip);

        #warning DELTA is disabled because it does not works with rdesktop
        uint8_t datas[] = {
            CHANGE | STANDARD | BOUNDS,
            GLYPHINDEX,
            0xff, 0xff, 0x3f, // fields
            0x1c, 0x05, 0x24, 0x03, 0x57, 0x02, // bounds
            0x01, // font cache_id
            0x20, // flags
            0x01, // charinc
            0x04, // f_op_redundant
            0x33, 0x22, 0x11, // back_color
            0x66, 0x55, 0x44, // fore_color
            0x01, 0x00, 0x02, 0x00, 0x28, 0x00, 0x3d, 0x00, // bk rect
            0x03, 0x00, 0x04, 0x00, 0x34, 0x00, 0x49, 0x00, // op rect
            0x03, 0x04, 0x03, 0xdd, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // brush
            0x05, 0x00, 0x06, 0x00,// glyph_x, glyph_y
            // 12, "Hello, World"
            0x0c, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64,
        };

        check_datas(stream.p-stream.data, stream.data, sizeof(datas), datas, "Text 1");
        stream.end = stream.p; stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)0x0D, header.control);
        BOOST_CHECK_EQUAL((uint32_t)0x3fffff, header.fields);
        BOOST_CHECK_EQUAL((uint8_t)GLYPHINDEX, common_cmd.order);
        BOOST_CHECK_EQUAL(Rect(5, 0, 800, 600), common_cmd.clip);

        RDPGlyphIndex cmd = statecmd;
        cmd.receive(stream, header);

        if (!(RDPBrush(3, 4, 0x03, 0xDD, (uint8_t*)"\1\2\3\4\5\6\7")
                == cmd.brush)){
            BOOST_CHECK_EQUAL(true, false);
        }
        check<RDPGlyphIndex>(common_cmd, cmd,
            RDPOrderCommon(GLYPHINDEX, Rect(5, 0, 800, 600)),
            RDPGlyphIndex(1, 0x20, 1, 4,
                          0x112233,
                          0x445566,
                          Rect(1,2,40,60),
                          Rect(3,4,50,70),
                          RDPBrush(3, 4, 0x03, 0xDD, (uint8_t*)"\1\2\3\4\5\6\7"),
                          5, 6,
     #warning actual data is much more complex, than a text, we should create a spécialized object to store, serialize and replay it. This should be done after the RDP layer includes cache management primitives
                          12, (uint8_t*)"Hello, World"),
            "Text 1");
    }


    {
        Stream stream(1000);

        RDPColCache newcmd(8);
        for (int i = 0; i < 256; ++i){
            newcmd.palette[0][i] = (((i >> 6) & 3) << 16) + (((i>>3) & 7) << 8) + (i & 7);
        }

        newcmd.emit(stream, 0);

        uint8_t datas[] = {
            STANDARD | SECONDARY,       // control
            0xfc, 0x03, // length
            0x00, 0x00, // flags
            0x01,       // type
            0x00,       // cacheIndex
            0x00, 0x01, // numberColors
            // data
            00, 00, 00, 00, 01, 00, 00, 00, 02, 00, 00, 00, 03, 00, 00, 00,
            04, 00, 00, 00, 05, 00, 00, 00, 06, 00, 00, 00, 07, 00, 00, 00,
            00, 01, 00, 00, 01, 01, 00, 00, 02, 01, 00, 00, 03, 01, 00, 00,
            04, 01, 00, 00, 05, 01, 00, 00, 06, 01, 00, 00, 07, 01, 00, 00,
            00, 02, 00, 00, 01, 02, 00, 00, 02, 02, 00, 00, 03, 02, 00, 00,
            04, 02, 00, 00, 05, 02, 00, 00, 06, 02, 00, 00, 07, 02, 00, 00,
            00, 03, 00, 00, 01, 03, 00, 00, 02, 03, 00, 00, 03, 03, 00, 00,
            04, 03, 00, 00, 05, 03, 00, 00, 06, 03, 00, 00, 07, 03, 00, 00,
            00, 04, 00, 00, 01, 04, 00, 00, 02, 04, 00, 00, 03, 04, 00, 00,
            04, 04, 00, 00, 05, 04, 00, 00, 06, 04, 00, 00, 07, 04, 00, 00,
            00, 05, 00, 00, 01, 05, 00, 00, 02, 05, 00, 00, 03, 05, 00, 00,
            04, 05, 00, 00, 05, 05, 00, 00, 06, 05, 00, 00, 07, 05, 00, 00,
            00, 06, 00, 00, 01, 06, 00, 00, 02, 06, 00, 00, 03, 06, 00, 00,
            04, 06, 00, 00, 05, 06, 00, 00, 06, 06, 00, 00, 07, 06, 00, 00,
            00, 07, 00, 00, 01, 07, 00, 00, 02, 07, 00, 00, 03, 07, 00, 00,
            04, 07, 00, 00, 05, 07, 00, 00, 06, 07, 00, 00, 07, 07, 00, 00,
            00, 00, 01, 00, 01, 00, 01, 00, 02, 00, 01, 00, 03, 00, 01, 00,
            04, 00, 01, 00, 05, 00, 01, 00, 06, 00, 01, 00, 07, 00, 01, 00,
            00, 01, 01, 00, 01, 01, 01, 00, 02, 01, 01, 00, 03, 01, 01, 00,
            04, 01, 01, 00, 05, 01, 01, 00, 06, 01, 01, 00, 07, 01, 01, 00,
            00, 02, 01, 00, 01, 02, 01, 00, 02, 02, 01, 00, 03, 02, 01, 00,
            04, 02, 01, 00, 05, 02, 01, 00, 06, 02, 01, 00, 07, 02, 01, 00,
            00, 03, 01, 00, 01, 03, 01, 00, 02, 03, 01, 00, 03, 03, 01, 00,
            04, 03, 01, 00, 05, 03, 01, 00, 06, 03, 01, 00, 07, 03, 01, 00,
            00, 04, 01, 00, 01, 04, 01, 00, 02, 04, 01, 00, 03, 04, 01, 00,
            04, 04, 01, 00, 05, 04, 01, 00, 06, 04, 01, 00, 07, 04, 01, 00,
            00, 05, 01, 00, 01, 05, 01, 00, 02, 05, 01, 00, 03, 05, 01, 00,
            04, 05, 01, 00, 05, 05, 01, 00, 06, 05, 01, 00, 07, 05, 01, 00,
            00, 06, 01, 00, 01, 06, 01, 00, 02, 06, 01, 00, 03, 06, 01, 00,
            04, 06, 01, 00, 05, 06, 01, 00, 06, 06, 01, 00, 07, 06, 01, 00,
            00, 07, 01, 00, 01, 07, 01, 00, 02, 07, 01, 00, 03, 07, 01, 00,
            04, 07, 01, 00, 05, 07, 01, 00, 06, 07, 01, 00, 07, 07, 01, 00,
            00, 00, 02, 00, 01, 00, 02, 00, 02, 00, 02, 00, 03, 00, 02, 00,
            04, 00, 02, 00, 05, 00, 02, 00, 06, 00, 02, 00, 07, 00, 02, 00,
            00, 01, 02, 00, 01, 01, 02, 00, 02, 01, 02, 00, 03, 01, 02, 00,
            04, 01, 02, 00, 05, 01, 02, 00, 06, 01, 02, 00, 07, 01, 02, 00,
            00, 02, 02, 00, 01, 02, 02, 00, 02, 02, 02, 00, 03, 02, 02, 00,
            04, 02, 02, 00, 05, 02, 02, 00, 06, 02, 02, 00, 07, 02, 02, 00,
            00, 03, 02, 00, 01, 03, 02, 00, 02, 03, 02, 00, 03, 03, 02, 00,
            04, 03, 02, 00, 05, 03, 02, 00, 06, 03, 02, 00, 07, 03, 02, 00,
            00, 04, 02, 00, 01, 04, 02, 00, 02, 04, 02, 00, 03, 04, 02, 00,
            04, 04, 02, 00, 05, 04, 02, 00, 06, 04, 02, 00, 07, 04, 02, 00,
            00, 05, 02, 00, 01, 05, 02, 00, 02, 05, 02, 00, 03, 05, 02, 00,
            04, 05, 02, 00, 05, 05, 02, 00, 06, 05, 02, 00, 07, 05, 02, 00,
            00, 06, 02, 00, 01, 06, 02, 00, 02, 06, 02, 00, 03, 06, 02, 00,
            04, 06, 02, 00, 05, 06, 02, 00, 06, 06, 02, 00, 07, 06, 02, 00,
            00, 07, 02, 00, 01, 07, 02, 00, 02, 07, 02, 00, 03, 07, 02, 00,
            04, 07, 02, 00, 05, 07, 02, 00, 06, 07, 02, 00, 07, 07, 02, 00,
            00, 00, 03, 00, 01, 00, 03, 00, 02, 00, 03, 00, 03, 00, 03, 00,
            04, 00, 03, 00, 05, 00, 03, 00, 06, 00, 03, 00, 07, 00, 03, 00,
            00, 01, 03, 00, 01, 01, 03, 00, 02, 01, 03, 00, 03, 01, 03, 00,
            04, 01, 03, 00, 05, 01, 03, 00, 06, 01, 03, 00, 07, 01, 03, 00,
            00, 02, 03, 00, 01, 02, 03, 00, 02, 02, 03, 00, 03, 02, 03, 00,
            04, 02, 03, 00, 05, 02, 03, 00, 06, 02, 03, 00, 07, 02, 03, 00,
            00, 03, 03, 00, 01, 03, 03, 00, 02, 03, 03, 00, 03, 03, 03, 00,
            04, 03, 03, 00, 05, 03, 03, 00, 06, 03, 03, 00, 07, 03, 03, 00,
            00, 04, 03, 00, 01, 04, 03, 00, 02, 04, 03, 00, 03, 04, 03, 00,
            04, 04, 03, 00, 05, 04, 03, 00, 06, 04, 03, 00, 07, 04, 03, 00,
            00, 05, 03, 00, 01, 05, 03, 00, 02, 05, 03, 00, 03, 05, 03, 00,
            04, 05, 03, 00, 05, 05, 03, 00, 06, 05, 03, 00, 07, 05, 03, 00,
            00, 06, 03, 00, 01, 06, 03, 00, 02, 06, 03, 00, 03, 06, 03, 00,
            04, 06, 03, 00, 05, 06, 03, 00, 06, 06, 03, 00, 07, 06, 03, 00,
            00, 07, 03, 00, 01, 07, 03, 00, 02, 07, 03, 00, 03, 07, 03, 00,
            04, 07, 03, 00, 05, 07, 03, 00, 06, 07, 03, 00, 07, 07, 03, 00,
        };

        check_datas(stream.p-stream.data, stream.data, sizeof(datas), datas, "Color Cache 1");
        stream.end = stream.p; stream.p = stream.data;

        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & (STANDARD|SECONDARY)));
        RDPSecondaryOrderHeader header(stream);
        RDPColCache cmd(8);
        cmd.receive(stream, control, header);

        check<RDPColCache>(cmd, newcmd, "Color Cache 1");
    }


}
