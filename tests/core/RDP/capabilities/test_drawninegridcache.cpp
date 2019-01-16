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

   Unit test to RDP DrawNineGridCache object
   Using lib boost functions for testing
*/
#define RED_TEST_MODULE CapabilityDrawNineGridCacheEmit

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/drawninegridcache.hpp"

RED_AUTO_TEST_CASE(TestCapabilityDrawNineGridCacheEmit)
{
    DrawNineGridCacheCaps drawninegridcache_caps;
    drawninegridcache_caps.drawNineGridSupportLevel = 0;
    drawninegridcache_caps.drawNineGridCacheSize = 2560;
    drawninegridcache_caps.drawNineGridCacheEntries = 256;

    RED_CHECK_EQUAL(drawninegridcache_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_DRAWNINEGRIDCACHE));
    RED_CHECK_EQUAL(drawninegridcache_caps.len, static_cast<uint16_t>(CAPLEN_DRAWNINEGRIDCACHE));
    RED_CHECK_EQUAL(drawninegridcache_caps.drawNineGridSupportLevel, static_cast<uint32_t>(0));
    RED_CHECK_EQUAL(drawninegridcache_caps.drawNineGridCacheSize, static_cast<uint16_t>(2560));
    RED_CHECK_EQUAL(drawninegridcache_caps.drawNineGridCacheEntries, static_cast<uint16_t>(256));

    StaticOutStream<1024> out_stream;
    drawninegridcache_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    DrawNineGridCacheCaps drawninegridcache_caps2;

    RED_CHECK_EQUAL(drawninegridcache_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_DRAWNINEGRIDCACHE));
    RED_CHECK_EQUAL(drawninegridcache_caps2.len, static_cast<uint16_t>(CAPLEN_DRAWNINEGRIDCACHE));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_DRAWNINEGRIDCACHE), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_DRAWNINEGRIDCACHE), stream.in_uint16_le());
    drawninegridcache_caps2.recv(stream, CAPLEN_DRAWNINEGRIDCACHE);

    RED_CHECK_EQUAL(drawninegridcache_caps2.drawNineGridSupportLevel, static_cast<uint32_t>(0));
    RED_CHECK_EQUAL(drawninegridcache_caps2.drawNineGridCacheSize, static_cast<uint16_t>(2560));
    RED_CHECK_EQUAL(drawninegridcache_caps2.drawNineGridCacheEntries, static_cast<uint16_t>(256));
}
