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

   Unit test to RDP Input object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityInput
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/input.hpp"

RED_AUTO_TEST_CASE(TestCapabilityInputEmit)
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

    RED_CHECK_EQUAL(input_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_INPUT));
    RED_CHECK_EQUAL(input_caps.len, static_cast<uint16_t>(CAPLEN_INPUT));
    RED_CHECK_EQUAL(input_caps.inputFlags, static_cast<uint16_t>(INPUT_FLAG_FASTPATH_INPUT2));
    RED_CHECK_EQUAL(input_caps.pad2octetsA, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(input_caps.keyboardLayout, static_cast<uint32_t>(2));
    RED_CHECK_EQUAL(input_caps.keyboardType, static_cast<uint32_t>(3));
    RED_CHECK_EQUAL(input_caps.keyboardSubType, static_cast<uint32_t>(4));
    RED_CHECK_EQUAL(input_caps.keyboardFunctionKey, static_cast<uint32_t>(5));
    for (size_t i = 0; i< 32; i++){
        RED_CHECK_EQUAL(input_caps.imeFileName[i], test_input[i]);
    }

    StaticOutStream<1024> out_stream;
    input_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    InputCaps input_caps2;

    RED_CHECK_EQUAL(input_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_INPUT));
    RED_CHECK_EQUAL(input_caps2.len, static_cast<uint16_t>(CAPLEN_INPUT));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_INPUT), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_INPUT), stream.in_uint16_le());
    input_caps2.recv(stream, CAPLEN_INPUT);

    RED_CHECK_EQUAL(input_caps2.inputFlags, static_cast<uint16_t>(INPUT_FLAG_FASTPATH_INPUT2));
    RED_CHECK_EQUAL(input_caps2.pad2octetsA, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(input_caps2.keyboardLayout, static_cast<uint32_t>(2));
    RED_CHECK_EQUAL(input_caps2.keyboardType, static_cast<uint32_t>(3));
    RED_CHECK_EQUAL(input_caps2.keyboardSubType, static_cast<uint32_t>(4));
    RED_CHECK_EQUAL(input_caps2.keyboardFunctionKey, static_cast<uint32_t>(5));
    for (size_t i = 0; i< 32; i++){
        RED_CHECK_EQUAL(input_caps2.imeFileName[i], test_input[i]);
    }
}
