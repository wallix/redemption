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

   Unit test to RDP Order object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityOrder
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include"log.hpp"
#include"RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityOrderEmit)
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

    BOOST_CHECK_EQUAL(order_caps.capabilityType, (uint16_t)CAPSTYPE_ORDER);
    BOOST_CHECK_EQUAL(order_caps.len, (uint16_t)CAPLEN_ORDER);
    for (size_t i = 0; i< 16; i++){
        BOOST_CHECK_EQUAL(order_caps.terminalDescriptor[i], test_order_td[i]);
    }
    BOOST_CHECK_EQUAL(order_caps.pad4octetsA, (uint32_t) 0);
    BOOST_CHECK_EQUAL(order_caps.desktopSaveXGranularity, (uint16_t) 1);
    BOOST_CHECK_EQUAL(order_caps.desktopSaveYGranularity, (uint16_t) 2);
    BOOST_CHECK_EQUAL(order_caps.pad2octetsA, (uint16_t) 3);
    BOOST_CHECK_EQUAL(order_caps.maximumOrderLevel, (uint16_t) 4);
    BOOST_CHECK_EQUAL(order_caps.numberFonts, (uint16_t) 5);
    BOOST_CHECK_EQUAL(order_caps.orderFlags, (uint16_t) 6);
    for (size_t i = 0; i< NB_ORDER_SUPPORT; i++){
        BOOST_CHECK_EQUAL(order_caps.orderSupport[i], test_order_os[i]);
    }
    BOOST_CHECK_EQUAL(order_caps.textFlags, (uint16_t) 7);
    BOOST_CHECK_EQUAL(order_caps.orderSupportExFlags, (uint16_t) 8);
    BOOST_CHECK_EQUAL(order_caps.pad4octetsB, (uint32_t) 9);
    BOOST_CHECK_EQUAL(order_caps.desktopSaveSize, (uint32_t) 10);
    BOOST_CHECK_EQUAL(order_caps.pad2octetsC, (uint16_t) 11);
    BOOST_CHECK_EQUAL(order_caps.pad2octetsD, (uint16_t) 12);
    BOOST_CHECK_EQUAL(order_caps.textANSICodePage, (uint16_t) 13);
    BOOST_CHECK_EQUAL(order_caps.pad2octetsE, (uint16_t) 14);

    BStream stream(1024);
    order_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.data;

    OrderCaps order_caps2;

    BOOST_CHECK_EQUAL(order_caps2.capabilityType, (uint16_t)CAPSTYPE_ORDER);
    BOOST_CHECK_EQUAL(order_caps2.len, (uint16_t)CAPLEN_ORDER);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_ORDER, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_ORDER, stream.in_uint16_le());
    order_caps2.recv(stream, CAPLEN_ORDER);

    for (size_t i = 0; i< 16; i++){
        BOOST_CHECK_EQUAL(order_caps2.terminalDescriptor[i], test_order_td[i]);
    }
    BOOST_CHECK_EQUAL(order_caps2.pad4octetsA, (uint32_t) 0);
    BOOST_CHECK_EQUAL(order_caps2.desktopSaveXGranularity, (uint16_t) 1);
    BOOST_CHECK_EQUAL(order_caps2.desktopSaveYGranularity, (uint16_t) 2);
    BOOST_CHECK_EQUAL(order_caps2.pad2octetsA, (uint16_t) 3);
    BOOST_CHECK_EQUAL(order_caps2.maximumOrderLevel, (uint16_t) 4);
    BOOST_CHECK_EQUAL(order_caps2.numberFonts, (uint16_t) 5);
    BOOST_CHECK_EQUAL(order_caps2.orderFlags, (uint16_t) 6);
    for (size_t i = 0; i< NB_ORDER_SUPPORT; i++){
        BOOST_CHECK_EQUAL(order_caps2.orderSupport[i], test_order_os[i]);
    }
    BOOST_CHECK_EQUAL(order_caps2.textFlags, (uint16_t) 7);
    BOOST_CHECK_EQUAL(order_caps2.orderSupportExFlags, (uint16_t) 8);
    BOOST_CHECK_EQUAL(order_caps2.pad4octetsB, (uint32_t) 9);
    BOOST_CHECK_EQUAL(order_caps2.desktopSaveSize, (uint32_t) 10);
    BOOST_CHECK_EQUAL(order_caps2.pad2octetsC, (uint16_t) 11);
    BOOST_CHECK_EQUAL(order_caps2.pad2octetsD, (uint16_t) 12);
    BOOST_CHECK_EQUAL(order_caps2.textANSICodePage, (uint16_t) 13);
    BOOST_CHECK_EQUAL(order_caps2.pad2octetsE, (uint16_t) 14);

}
