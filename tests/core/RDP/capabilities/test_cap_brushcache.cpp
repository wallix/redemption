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

   Unit test to RDP BrushCache object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityBrushCache
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/capabilities/cap_brushcache.hpp"

RED_AUTO_TEST_CASE(TestCapabilityBrushCacheEmit)
{
    BrushCacheCaps brushcache_caps;
    brushcache_caps.brushSupportLevel = BRUSH_COLOR_8X8;

    RED_CHECK_EQUAL(brushcache_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_BRUSH));
    RED_CHECK_EQUAL(brushcache_caps.len, static_cast<uint16_t>(CAPLEN_BRUSH));
    RED_CHECK_EQUAL(brushcache_caps.brushSupportLevel, static_cast<uint32_t>(BRUSH_COLOR_8X8));

    StaticOutStream<1024> out_stream;
    brushcache_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());


    BrushCacheCaps brushcache_caps2;

    RED_CHECK_EQUAL(brushcache_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_BRUSH));
    RED_CHECK_EQUAL(brushcache_caps2.len, static_cast<uint16_t>(CAPLEN_BRUSH));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_BRUSH), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_BRUSH), stream.in_uint16_le());
    brushcache_caps2.recv(stream, CAPLEN_BRUSH);

    RED_CHECK_EQUAL(brushcache_caps2.brushSupportLevel, static_cast<uint32_t>(BRUSH_COLOR_8X8));
}
