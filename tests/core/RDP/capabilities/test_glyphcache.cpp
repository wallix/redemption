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

   Unit test to RDP GlyphCache object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityGlyphSupport
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilityGlyphSupportEmit)
{
    GlyphSupportCaps glyphsupport_caps;

    for (uint8_t i = 0; i< 40; i++){
        glyphsupport_caps.glyphCache[i] = i;
    }
    glyphsupport_caps.fragCache = 16777216;
    glyphsupport_caps.glyphSupportLevel = GLYPH_SUPPORT_FULL;
    glyphsupport_caps.pad2octets = 3;

    uint8_t test_glyphCache[40];
    for (uint8_t i = 0; i< 40; i++){
        test_glyphCache[i] = i;
    }
    BOOST_CHECK_EQUAL(glyphsupport_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_GLYPHCACHE));
    BOOST_CHECK_EQUAL(glyphsupport_caps.len, static_cast<uint16_t>(CAPLEN_GLYPHCACHE));
    for (size_t i = 0; i< 40; i++){
        BOOST_CHECK_EQUAL(glyphsupport_caps.glyphCache[i], test_glyphCache[i]);
    }
    BOOST_CHECK_EQUAL(glyphsupport_caps.fragCache, (uint32_t) 16777216);
    BOOST_CHECK_EQUAL(glyphsupport_caps.glyphSupportLevel, (uint16_t) GLYPH_SUPPORT_FULL);
    BOOST_CHECK_EQUAL(glyphsupport_caps.pad2octets, (uint16_t) 3);

    BStream stream(1024);
    glyphsupport_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.get_data();

    GlyphSupportCaps glyphsupport_caps2;

    BOOST_CHECK_EQUAL(glyphsupport_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_GLYPHCACHE));
    BOOST_CHECK_EQUAL(glyphsupport_caps2.len, static_cast<uint16_t>(CAPLEN_GLYPHCACHE));

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_GLYPHCACHE, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_GLYPHCACHE, stream.in_uint16_le());
    glyphsupport_caps2.recv(stream, CAPLEN_GLYPHCACHE);

    for (size_t i = 0; i< 40; i++){
        BOOST_CHECK_EQUAL(glyphsupport_caps2.glyphCache[i], test_glyphCache[i]);
    }
    BOOST_CHECK_EQUAL(glyphsupport_caps2.fragCache, (uint32_t) 16777216);
    BOOST_CHECK_EQUAL(glyphsupport_caps2.glyphSupportLevel, (uint16_t) GLYPH_SUPPORT_FULL);
    BOOST_CHECK_EQUAL(glyphsupport_caps2.pad2octets, (uint16_t) 3);
}
