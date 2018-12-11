/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    Unit test to RDP GlyphCache object
    Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityGlyphSupport
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/cap_glyphcache.hpp"

RED_AUTO_TEST_CASE(TestCapabilityGlyphSupportEmit)
{
    GlyphCacheCaps glyphcache_caps;

    for (uint8_t i = 0; i < GlyphCacheCaps::NUMBER_OF_CACHE; ++i) {
        glyphcache_caps.GlyphCache[i].CacheEntries         = 128;
        glyphcache_caps.GlyphCache[i].CacheMaximumCellSize = 256;
    }
    glyphcache_caps.FragCache         = 16777216;
    glyphcache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_FULL;
    glyphcache_caps.pad2octets        = 3;

    RED_CHECK_EQUAL(glyphcache_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_GLYPHCACHE));
    RED_CHECK_EQUAL(glyphcache_caps.len,            GlyphCacheCaps::LENGTH_CAPABILITY);

    StaticOutStream<1024> out_stream;
    glyphcache_caps.emit(out_stream);

    RED_CHECK_EQUAL(out_stream.get_offset(), GlyphCacheCaps::LENGTH_CAPABILITY);

    InStream stream(out_stream.get_bytes());

    GlyphCacheCaps glyphcache_caps2;

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_GLYPHCACHE),               stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(GlyphCacheCaps::LENGTH_CAPABILITY), stream.in_uint16_le());

    glyphcache_caps2.recv(stream, GlyphCacheCaps::LENGTH_CAPABILITY);

    for (uint8_t i = 0; i < GlyphCacheCaps::NUMBER_OF_CACHE; ++i) {
        RED_CHECK_EQUAL(glyphcache_caps2.GlyphCache[i].CacheEntries,         128);
        RED_CHECK_EQUAL(glyphcache_caps2.GlyphCache[i].CacheMaximumCellSize, 256);
    }
    RED_CHECK_EQUAL(glyphcache_caps2.FragCache,         16777216);
    RED_CHECK_EQUAL(glyphcache_caps2.GlyphSupportLevel, static_cast<uint16_t>(GlyphCacheCaps::GLYPH_SUPPORT_FULL));
    RED_CHECK_EQUAL(glyphcache_caps2.pad2octets,        3);
}
