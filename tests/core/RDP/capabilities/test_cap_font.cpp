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

   Unit test to RDP Font object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityFont
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilityFontEmit)
{
    FontCaps font_caps;
    font_caps.fontSupportFlags = 32769;
    font_caps.pad2octets = 65535;

    BOOST_CHECK_EQUAL(font_caps.capabilityType, (uint16_t)CAPSTYPE_FONT);
    BOOST_CHECK_EQUAL(font_caps.len, (uint16_t)CAPLEN_FONT);
    BOOST_CHECK_EQUAL(font_caps.fontSupportFlags, (uint16_t) 32769);
    BOOST_CHECK_EQUAL(font_caps.pad2octets, (uint16_t) 65535);

    BStream stream(1024);
    font_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.get_data();

    FontCaps font_caps2;

    BOOST_CHECK_EQUAL(font_caps2.capabilityType, (uint16_t)CAPSTYPE_FONT);
    BOOST_CHECK_EQUAL(font_caps2.len, (uint16_t)CAPLEN_FONT);
    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_FONT, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_FONT, stream.in_uint16_le());
    font_caps2.recv(stream, CAPLEN_FONT);

    BOOST_CHECK_EQUAL(font_caps2.fontSupportFlags, (uint16_t) 32769);
    BOOST_CHECK_EQUAL(font_caps2.pad2octets, (uint16_t) 65535);
}
