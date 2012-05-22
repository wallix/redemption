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

   Unit test to RDP BrushCache object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityBrushCache
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityBrushCacheEmit)
{
    BrushCacheCaps brushcache_caps;
    brushcache_caps.brushSupportLevel = BRUSH_COLOR_8X8;

    BOOST_CHECK_EQUAL(brushcache_caps.capabilityType, (uint16_t)CAPSTYPE_BRUSH);
    BOOST_CHECK_EQUAL(brushcache_caps.len, (uint16_t)RDP_CAPLEN_BRUSH);
    BOOST_CHECK_EQUAL(brushcache_caps.brushSupportLevel, (uint32_t)BRUSH_COLOR_8X8);

    Stream stream(1024);
    brushcache_caps.emit(stream);
    stream.end = stream.p;
    stream.p = stream.data;

    BrushCacheCaps brushcache_caps2;

    BOOST_CHECK_EQUAL(brushcache_caps2.capabilityType, (uint16_t)CAPSTYPE_BRUSH);
    BOOST_CHECK_EQUAL(brushcache_caps2.len, (uint16_t)RDP_CAPLEN_BRUSH);
    brushcache_caps2.recv(stream);

    BOOST_CHECK_EQUAL(brushcache_caps2.brushSupportLevel, (uint32_t)BRUSH_COLOR_8X8);
}
