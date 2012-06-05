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

   Unit test to RDP Rail object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilityRailEmit)
{
    RailCaps rail_caps;
    rail_caps.RailSupportLevel = true;

    BOOST_CHECK_EQUAL(rail_caps.capabilityType, (uint16_t)CAPSTYPE_RAIL);
    BOOST_CHECK_EQUAL(rail_caps.len, (uint16_t)RDP_CAPLEN_RAIL);
    BOOST_CHECK_EQUAL(rail_caps.RailSupportLevel, (uint32_t) 1);

    Stream stream(1024);
    rail_caps.emit(stream);
    stream.end = stream.p;
    stream.p = stream.data;

    RailCaps rail_caps2;

    BOOST_CHECK_EQUAL(rail_caps2.capabilityType, (uint16_t)CAPSTYPE_RAIL);
    BOOST_CHECK_EQUAL(rail_caps2.len, (uint16_t)RDP_CAPLEN_RAIL);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_RAIL, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)RDP_CAPLEN_RAIL, stream.in_uint16_le());
    rail_caps2.recv(stream, RDP_CAPLEN_RAIL);

    BOOST_CHECK_EQUAL(rail_caps2.RailSupportLevel, (uint32_t) 1);
}
