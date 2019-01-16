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

   Unit test to RDP OffScreenCache object
   Using lib boost functions for testing
*/
#define RED_TEST_MODULE CapabilityOffScreenCacheEmit

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/offscreencache.hpp"

RED_AUTO_TEST_CASE(TestCapabilityOffScreenCacheEmit)
{
    OffScreenCacheCaps offscreencache_caps;
    offscreencache_caps.offscreenSupportLevel = true;
    offscreencache_caps.offscreenCacheSize = 7680;
    offscreencache_caps.offscreenCacheEntries = 500;

    RED_CHECK_EQUAL(offscreencache_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_OFFSCREENCACHE));
    RED_CHECK_EQUAL(offscreencache_caps.len, static_cast<uint16_t>(CAPLEN_OFFSCREENCACHE));
    RED_CHECK_EQUAL(offscreencache_caps.offscreenSupportLevel, static_cast<uint32_t>(1));
    RED_CHECK_EQUAL(offscreencache_caps.offscreenCacheSize, static_cast<uint16_t>(7680));
    RED_CHECK_EQUAL(offscreencache_caps.offscreenCacheEntries, static_cast<uint16_t>(500));

    StaticOutStream<1024> out_stream;
    offscreencache_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    OffScreenCacheCaps offscreencache_caps2;

    RED_CHECK_EQUAL(offscreencache_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_OFFSCREENCACHE));
    RED_CHECK_EQUAL(offscreencache_caps2.len, static_cast<uint16_t>(CAPLEN_OFFSCREENCACHE));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_OFFSCREENCACHE), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_OFFSCREENCACHE), stream.in_uint16_le());
    offscreencache_caps2.recv(stream, CAPLEN_OFFSCREENCACHE);

    RED_CHECK_EQUAL(offscreencache_caps2.offscreenSupportLevel, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(offscreencache_caps2.offscreenCacheSize, static_cast<uint16_t>(7680));
    RED_CHECK_EQUAL(offscreencache_caps2.offscreenCacheEntries, static_cast<uint16_t>(500));
}
