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
#define BOOST_TEST_MODULE TestOrderEllipseCB
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"

#include "test_orders.hpp"

BOOST_AUTO_TEST_CASE(TestEllipseCB)
{
    using namespace RDP;

    {
        // 0x40: Back color (3 bytes)
        // 0x80: Fore color (3 bytes)
        // 0x100: Brush Org X (1 byte)

        // 0x0200: Brush Org Y (1 byte)
        // 0x0400: Brush style (1 byte)
        // 0x0800: Brush Hatch (1 byte) + Extra if style == 0x3

        StaticOutStream<1000> out_stream;

        // DESTBLT = 0, hence we won't have order change
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPEllipseCB state_ellipse(Rect(), 0, 0x00, 0, 0, RDPBrush());
        BOOST_CHECK(state_ellipse.id() == ELLIPSECB);
        //state_ellipse.log(1, Rect());
        //state_ellipse.print(Rect());
        RDPOrderCommon newcommon(ELLIPSECB, Rect(311, 0, 800, 600));
        RDPEllipseCB(Rect(300, 400, 50, 60),
                     0xFF, 0x01,
                     0x102030, 0x112233,
                     RDPBrush(3, 4, 3, 0xDD, reinterpret_cast<const uint8_t*>("\1\2\3\4\5\6\7"))
                     ).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[31] = {
            0x2d, 0x1a,
            0xff, 0x1f,
            0x2c, 0x01,
            0x90, 0x01,
            0x5e, 0x01,
            0xcc, 0x01,
            0xff, 0x01, 0x30,
            0x20, 0x10,
            0x33, 0x22,
            0x11, 0x03,
            0x04, 0x03,
            0xdd, 0x01,
            0x02, 0x03,
            0x04, 0x05,
            0x06, 0x07 };
        check_datas(out_stream.get_offset(), out_stream.get_data(), 31, datas, "EllipseCB 1");

        InStream in_stream(out_stream.get_data(), out_stream.get_offset());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        BOOST_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSECB), common_cmd.order);

        RDPEllipseCB cmd(Rect(), 0, 0x00, 0, 0, RDPBrush());

        cmd.receive(in_stream, header);

        check<RDPEllipseCB>(common_cmd, cmd,
                            RDPOrderCommon(ELLIPSECB, Rect(311, 0, 800, 600)),
                            RDPEllipseCB(Rect(300, 400, 50, 60), 0xFF, 0x01, 0x102030, 0x112233,
                                         RDPBrush(3, 4, 0x03, 0xDD,
                                         reinterpret_cast<const uint8_t*>("\1\2\3\4\5\6\7"))),
                            "EllipseCB 1");
    }

    {
        StaticOutStream<1000> out_stream;

        // DESTBLT = 0, hence we won't have order change
        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPEllipseCB state_ellipse(Rect(), 0, 0x01, 0, 0, RDPBrush());

        RDPOrderCommon newcommon(ELLIPSECB, Rect(311, 0, 800, 600));
        RDPEllipseCB(Rect(300, 400, 50, 60),
                     0xFF, 0x01,
                     0x102030, 0x112233,
                     RDPBrush(3, 4, 1, 0xDD)
                     ).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[23] =
            { 0x2d, 0x1a,
              0xdf, 0x0f, 0x2c, 0x01, 0x90, 0x01, 0x5e, 0x01,
              0xcc, 0x01, 0xff, 0x30, 0x20, 0x10,
              0x33, 0x22, 0x11, 0x03, 0x04, 0x01, 0xdd };

        check_datas(out_stream.get_offset(), out_stream.get_data(), 23, datas, "EllipseCB 2");

        InStream in_stream(out_stream.get_data(), out_stream.get_offset());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        BOOST_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSECB), common_cmd.order);

        RDPEllipseCB cmd(Rect(), 0, 0x01, 0, 0, RDPBrush());

        cmd.receive(in_stream, header);

        check<RDPEllipseCB>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSECB, Rect(311, 0, 800, 600)),
            RDPEllipseCB(Rect(300, 400, 50, 60), 0xFF, 0x01, 0x102030, 0x112233,
                RDPBrush(3, 4, 0x01, 0xDD)),
            "EllipseCB 2");
    }

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPEllipseCB state_ellipse(Rect(), 0, 0x01, 0, 0, RDPBrush(0, 0, 0x03, 0xDD));

        RDPOrderCommon newcommon(ELLIPSECB, Rect(311, 0, 800, 600));

        RDPEllipseCB(Rect(300, 400, 50, 60),
                  0xFF, 0x01,
                  0x102030, 0x112233,
                  RDPBrush(3, 4, 3, 0xDD, reinterpret_cast<const uint8_t*>("\1\2\3\4\5\6\7"))
                  ).emit(out_stream, newcommon, state_common, state_ellipse);

        uint8_t datas[28] =
            { 0x2d, 0x1a,
              0xdf, 0x13, 0x2c, 0x01, 0x90, 0x01, 0x5e, 0x01,
              0xcc, 0x01, 0xff, 0x30, 0x20, 0x10, 0x33, 0x22, 0x11, 0x03, 0x04,
              0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
        check_datas(out_stream.get_offset(), out_stream.get_data(), 28, datas, "EllipseCB 3");

        InStream in_stream(out_stream.get_data(), out_stream.get_offset());

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        BOOST_CHECK_EQUAL(static_cast<uint8_t>(ELLIPSECB), common_cmd.order);

        RDPEllipseCB cmd(Rect(), 0, 0x01, 0, 0, RDPBrush(0, 0, 3, 0xDD));

        cmd.receive(in_stream, header);

        check<RDPEllipseCB>(common_cmd, cmd,
            RDPOrderCommon(ELLIPSECB, Rect(311, 0, 800, 600)),
            RDPEllipseCB(Rect(300, 400, 50, 60), 0xFF, 0x01, 0x102030, 0x112233,
                RDPBrush(3, 4, 0x03, 0xDD, reinterpret_cast<const uint8_t*>("\1\2\3\4\5\6\7"))),
            "EllipseCB 3");
    }

}

