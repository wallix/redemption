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

   Unit test to RDP BmpCache object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityBmpCache
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/capabilities/cap_bmpcache.hpp"

RED_AUTO_TEST_CASE(TestCapabilityBmpCachepEmit)
{
    BmpCacheCaps bmpcache_caps;
    bmpcache_caps.pad1 = 1;
    bmpcache_caps.pad2 = 2;
    bmpcache_caps.pad3 = 3;
    bmpcache_caps.pad4 = 4;
    bmpcache_caps.pad5 = 5;
    bmpcache_caps.pad6 = 6;
    bmpcache_caps.cache0Entries = 200;
    bmpcache_caps.cache0MaximumCellSize = 32769;
    bmpcache_caps.cache1Entries = 600;
    bmpcache_caps.cache1MaximumCellSize = 32769;
    bmpcache_caps.cache2Entries = 65535;
    bmpcache_caps.cache2MaximumCellSize = 32769;

    RED_CHECK_EQUAL(bmpcache_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_BITMAPCACHE));
    RED_CHECK_EQUAL(bmpcache_caps.len, static_cast<uint16_t>(CAPLEN_BITMAPCACHE));
    RED_CHECK_EQUAL(bmpcache_caps.pad1, static_cast<uint32_t>(1));
    RED_CHECK_EQUAL(bmpcache_caps.pad2, static_cast<uint32_t>(2));
    RED_CHECK_EQUAL(bmpcache_caps.pad3, static_cast<uint32_t>(3));
    RED_CHECK_EQUAL(bmpcache_caps.pad4, static_cast<uint32_t>(4));
    RED_CHECK_EQUAL(bmpcache_caps.pad5, static_cast<uint32_t>(5));
    RED_CHECK_EQUAL(bmpcache_caps.pad6, static_cast<uint32_t>(6));
    RED_CHECK_EQUAL(bmpcache_caps.cache0Entries, static_cast<uint16_t>(200));
    RED_CHECK_EQUAL(bmpcache_caps.cache0MaximumCellSize, static_cast<uint16_t>(32769));
    RED_CHECK_EQUAL(bmpcache_caps.cache1Entries, static_cast<uint16_t>(600));
    RED_CHECK_EQUAL(bmpcache_caps.cache1MaximumCellSize, static_cast<uint16_t>(32769));
    RED_CHECK_EQUAL(bmpcache_caps.cache2Entries, static_cast<uint16_t>(65535));
    RED_CHECK_EQUAL(bmpcache_caps.cache2MaximumCellSize, static_cast<uint16_t>(32769));

    StaticOutStream<1024> out_stream;
    bmpcache_caps.emit(out_stream);
    InStream stream(out_stream.get_bytes());

    BmpCacheCaps bmpcache_caps2;

    RED_CHECK_EQUAL(bmpcache_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_BITMAPCACHE));
    RED_CHECK_EQUAL(bmpcache_caps2.len, static_cast<uint16_t>(CAPLEN_BITMAPCACHE));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_BITMAPCACHE), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_BITMAPCACHE), stream.in_uint16_le());
    bmpcache_caps2.recv(stream, CAPLEN_BITMAPCACHE);

    RED_CHECK_EQUAL(bmpcache_caps2.pad1, static_cast<uint32_t>(1));
    RED_CHECK_EQUAL(bmpcache_caps2.pad2, static_cast<uint32_t>(2));
    RED_CHECK_EQUAL(bmpcache_caps2.pad3, static_cast<uint32_t>(3));
    RED_CHECK_EQUAL(bmpcache_caps2.pad4, static_cast<uint32_t>(4));
    RED_CHECK_EQUAL(bmpcache_caps2.pad5, static_cast<uint32_t>(5));
    RED_CHECK_EQUAL(bmpcache_caps2.pad6, static_cast<uint32_t>(6));
    RED_CHECK_EQUAL(bmpcache_caps2.cache0Entries, static_cast<uint16_t>(200));
    RED_CHECK_EQUAL(bmpcache_caps2.cache0MaximumCellSize, static_cast<uint16_t>(32769));
    RED_CHECK_EQUAL(bmpcache_caps2.cache1Entries, static_cast<uint16_t>(600));
    RED_CHECK_EQUAL(bmpcache_caps2.cache1MaximumCellSize, static_cast<uint16_t>(32769));
    RED_CHECK_EQUAL(bmpcache_caps2.cache2Entries, static_cast<uint16_t>(65535));
    RED_CHECK_EQUAL(bmpcache_caps2.cache2MaximumCellSize, static_cast<uint16_t>(32769));
}
