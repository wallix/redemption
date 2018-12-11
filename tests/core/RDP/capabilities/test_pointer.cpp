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

   Unit test to RDP Pointer object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityPointer
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/pointer.hpp"

RED_AUTO_TEST_CASE(TestCapabilityPointerEmit)
{
    PointerCaps pointer_caps;
    pointer_caps.colorPointerFlag = 0;
    pointer_caps.colorPointerCacheSize = 1;
    pointer_caps.pointerCacheSize = 2;

    RED_CHECK_EQUAL(pointer_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_POINTER));
    RED_CHECK_EQUAL(pointer_caps.len, static_cast<uint16_t>(CAPLEN_POINTER));
    RED_CHECK_EQUAL(pointer_caps.colorPointerFlag, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(pointer_caps.colorPointerCacheSize, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(pointer_caps.pointerCacheSize, static_cast<uint16_t>(2));

    StaticOutStream<1024> out_stream;
    pointer_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    PointerCaps pointer_caps2;

    RED_CHECK_EQUAL(pointer_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_POINTER));
    RED_CHECK_EQUAL(pointer_caps2.len, static_cast<uint16_t>(CAPLEN_POINTER));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_POINTER), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_POINTER), stream.in_uint16_le());
    pointer_caps2.recv(stream, CAPLEN_POINTER);

    RED_CHECK_EQUAL(pointer_caps2.colorPointerFlag, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(pointer_caps2.colorPointerCacheSize, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(pointer_caps2.pointerCacheSize, static_cast<uint16_t>(2));
}
