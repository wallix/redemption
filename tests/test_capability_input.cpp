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

   Unit test to RDP Input object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityInput
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityInputEmit)
{
    InputCaps input_caps;
    input_caps.inputFlags = INPUT_FLAG_FASTPATH_INPUT2;
    input_caps.pad2octetsA = 1;
    input_caps.keyboardLayout = 2;
    input_caps.keyboardType = 3;
    input_caps.keyboardSubType = 4;
    input_caps.keyboardFunctionKey = 5;
    for (uint16_t i = 0; i< 32; i++){
        input_caps.imeFileName[i] = i;
    }

    uint8_t test_input[32];
    for (uint16_t i = 0; i< 32; i++){
        test_input[i] = i;
    }

    BOOST_CHECK_EQUAL(input_caps.capabilityType, (uint16_t)CAPSTYPE_INPUT);
    BOOST_CHECK_EQUAL(input_caps.len, (uint16_t)RDP_CAPLEN_INPUT);
    BOOST_CHECK_EQUAL(input_caps.inputFlags, (uint16_t) INPUT_FLAG_FASTPATH_INPUT2);
    BOOST_CHECK_EQUAL(input_caps.pad2octetsA, (uint16_t) 1);
    BOOST_CHECK_EQUAL(input_caps.keyboardLayout, (uint32_t) 2);
    BOOST_CHECK_EQUAL(input_caps.keyboardType, (uint32_t) 3);
    BOOST_CHECK_EQUAL(input_caps.keyboardSubType, (uint32_t) 4);
    BOOST_CHECK_EQUAL(input_caps.keyboardFunctionKey, (uint32_t) 5);
    for (size_t i = 0; i< 32; i++){
        BOOST_CHECK_EQUAL(input_caps.imeFileName[i], test_input[i]);
    }

    Stream stream(1024);
    input_caps.emit(stream);
    stream.end = stream.p;
    stream.p = stream.data;

    InputCaps input_caps2;

    BOOST_CHECK_EQUAL(input_caps2.capabilityType, (uint16_t)CAPSTYPE_INPUT);
    BOOST_CHECK_EQUAL(input_caps2.len, (uint16_t)RDP_CAPLEN_INPUT);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_INPUT, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)RDP_CAPLEN_INPUT, stream.in_uint16_le());
    input_caps2.recv(stream, RDP_CAPLEN_INPUT);

    BOOST_CHECK_EQUAL(input_caps2.inputFlags, (uint16_t) INPUT_FLAG_FASTPATH_INPUT2);
    BOOST_CHECK_EQUAL(input_caps2.pad2octetsA, (uint16_t) 1);
    BOOST_CHECK_EQUAL(input_caps2.keyboardLayout, (uint32_t) 2);
    BOOST_CHECK_EQUAL(input_caps2.keyboardType, (uint32_t) 3);
    BOOST_CHECK_EQUAL(input_caps2.keyboardSubType, (uint32_t) 4);
    BOOST_CHECK_EQUAL(input_caps2.keyboardFunctionKey, (uint32_t) 5);
    for (size_t i = 0; i< 32; i++){
        BOOST_CHECK_EQUAL(input_caps2.imeFileName[i], test_input[i]);
    }
}
