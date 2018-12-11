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

   Unit test to RDP Order object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityOrder
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/order.hpp"
#include "utils/hexdump.hpp"

RED_AUTO_TEST_CASE(TestCapabilityOrderEmit)
{
    OrderCaps order_caps;
    for (uint8_t i = 0; i< 16; i++){
        order_caps.terminalDescriptor[i] = i;
    }
    order_caps.pad4octetsA = 0;
    order_caps.desktopSaveXGranularity = 1;
    order_caps.desktopSaveYGranularity = 2;
    order_caps.pad2octetsA = 3;
    order_caps.maximumOrderLevel = 4;
    order_caps.numberFonts = 5;
    order_caps.orderFlags = 6;
    for (uint8_t i = 0; i< NB_ORDER_SUPPORT; i++){
        order_caps.orderSupport[i] = i;
    }
    order_caps.textFlags = 7;
    order_caps.orderSupportExFlags = 8;
    order_caps.pad4octetsB = 9;
    order_caps.desktopSaveSize = 10;
    order_caps.pad2octetsC = 11;
    order_caps.pad2octetsD = 12;
    order_caps.textANSICodePage = 13;
    order_caps.pad2octetsE = 14;

    uint8_t test_order_td[16];
    for (uint8_t i = 0; i< 16; i++){
        test_order_td[i] = i;
    }
    uint8_t test_order_os[NB_ORDER_SUPPORT];
    for (uint8_t i = 0; i< NB_ORDER_SUPPORT; i++){
        test_order_os[i] = i;
    }

    RED_CHECK_EQUAL(order_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_ORDER));
    RED_CHECK_EQUAL(order_caps.len, static_cast<uint16_t>(CAPLEN_ORDER));
    for (size_t i = 0; i< 16; i++){
        RED_CHECK_EQUAL(order_caps.terminalDescriptor[i], test_order_td[i]);
    }
    RED_CHECK_EQUAL(order_caps.pad4octetsA, static_cast<uint32_t>(0));
    RED_CHECK_EQUAL(order_caps.desktopSaveXGranularity, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(order_caps.desktopSaveYGranularity, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(order_caps.pad2octetsA, static_cast<uint16_t>(3));
    RED_CHECK_EQUAL(order_caps.maximumOrderLevel, static_cast<uint16_t>(4));
    RED_CHECK_EQUAL(order_caps.numberFonts, static_cast<uint16_t>(5));
    RED_CHECK_EQUAL(order_caps.orderFlags, static_cast<uint16_t>(6));
    for (size_t i = 0; i< NB_ORDER_SUPPORT; i++){
        RED_CHECK_EQUAL(order_caps.orderSupport[i], test_order_os[i]);
    }
    RED_CHECK_EQUAL(order_caps.textFlags, static_cast<uint16_t>(7));
    RED_CHECK_EQUAL(order_caps.orderSupportExFlags, static_cast<uint16_t>(8));
    RED_CHECK_EQUAL(order_caps.pad4octetsB, static_cast<uint32_t>(9));
    RED_CHECK_EQUAL(order_caps.desktopSaveSize, static_cast<uint32_t>(10));
    RED_CHECK_EQUAL(order_caps.pad2octetsC, static_cast<uint16_t>(11));
    RED_CHECK_EQUAL(order_caps.pad2octetsD, static_cast<uint16_t>(12));
    RED_CHECK_EQUAL(order_caps.textANSICodePage, static_cast<uint16_t>(13));
    RED_CHECK_EQUAL(order_caps.pad2octetsE, static_cast<uint16_t>(14));

    StaticOutStream<1024> out_stream;
    order_caps.emit(out_stream);

    hexdump_av_d(out_stream.get_bytes());

    InStream stream(out_stream.get_bytes());

    OrderCaps order_caps2;

    RED_CHECK_EQUAL(order_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_ORDER));
    RED_CHECK_EQUAL(order_caps2.len, static_cast<uint16_t>(CAPLEN_ORDER));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_ORDER), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_ORDER), stream.in_uint16_le());
    order_caps2.recv(stream, CAPLEN_ORDER);

    for (size_t i = 0; i< 16; i++){
        RED_CHECK_EQUAL(order_caps2.terminalDescriptor[i], test_order_td[i]);
    }
    RED_CHECK_EQUAL(order_caps2.pad4octetsA, static_cast<uint32_t>(0));
    RED_CHECK_EQUAL(order_caps2.desktopSaveXGranularity, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(order_caps2.desktopSaveYGranularity, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(order_caps2.pad2octetsA, static_cast<uint16_t>(3));
    RED_CHECK_EQUAL(order_caps2.maximumOrderLevel, static_cast<uint16_t>(4));
    RED_CHECK_EQUAL(order_caps2.numberFonts, static_cast<uint16_t>(5));
    RED_CHECK_EQUAL(order_caps2.orderFlags, static_cast<uint16_t>(6));
    for (size_t i = 0; i< NB_ORDER_SUPPORT; i++){
        RED_CHECK_EQUAL(order_caps2.orderSupport[i], test_order_os[i]);
    }
    RED_CHECK_EQUAL(order_caps2.textFlags, static_cast<uint16_t>(7));
    RED_CHECK_EQUAL(order_caps2.orderSupportExFlags, static_cast<uint16_t>(8));
    RED_CHECK_EQUAL(order_caps2.pad4octetsB, static_cast<uint32_t>(9));
    RED_CHECK_EQUAL(order_caps2.desktopSaveSize, static_cast<uint32_t>(10));
    RED_CHECK_EQUAL(order_caps2.pad2octetsC, static_cast<uint16_t>(11));
    RED_CHECK_EQUAL(order_caps2.pad2octetsD, static_cast<uint16_t>(12));
    RED_CHECK_EQUAL(order_caps2.textANSICodePage, static_cast<uint16_t>(13));
    RED_CHECK_EQUAL(order_caps2.pad2octetsE, static_cast<uint16_t>(14));
}
