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

   Unit test to RDP Pointer object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityPointer
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityPointerEmit)
{
    PointerCaps pointer_caps;
    pointer_caps.colorPointerFlag = 0;
    pointer_caps.colorPointerCacheSize = 1;
    pointer_caps.pointerCacheSize = 2;

    BOOST_CHECK_EQUAL(pointer_caps.capabilityType, (uint16_t)CAPSTYPE_POINTER);
    BOOST_CHECK_EQUAL(pointer_caps.len, (uint16_t)CAPLEN_POINTER);
    BOOST_CHECK_EQUAL(pointer_caps.colorPointerFlag, (uint16_t) 0);
    BOOST_CHECK_EQUAL(pointer_caps.colorPointerCacheSize, (uint16_t) 1);
    BOOST_CHECK_EQUAL(pointer_caps.pointerCacheSize, (uint16_t) 2);

    Stream stream(1024);
    pointer_caps.emit(stream);
    stream.end = stream.p;
    stream.p = stream.data;

    PointerCaps pointer_caps2;

    BOOST_CHECK_EQUAL(pointer_caps2.capabilityType, (uint16_t)CAPSTYPE_POINTER);
    BOOST_CHECK_EQUAL(pointer_caps2.len, (uint16_t)CAPLEN_POINTER);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_POINTER, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_POINTER, stream.in_uint16_le());
    pointer_caps2.recv(stream, CAPLEN_POINTER);

    BOOST_CHECK_EQUAL(pointer_caps2.colorPointerFlag, (uint16_t) 0);
    BOOST_CHECK_EQUAL(pointer_caps2.colorPointerCacheSize, (uint16_t) 1);
    BOOST_CHECK_EQUAL(pointer_caps2.pointerCacheSize, (uint16_t) 2);
}
