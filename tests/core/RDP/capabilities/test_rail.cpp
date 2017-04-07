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

   Unit test to RDP Rail object
   Using lib boost functions for testing
*/

#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/RDP/capabilities/rail.hpp"

RED_AUTO_TEST_CASE(TestCapabilityRailEmit)
{
    RailCaps rail_caps;
    rail_caps.RailSupportLevel = true;

    RED_CHECK_EQUAL(rail_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_RAIL));
    RED_CHECK_EQUAL(rail_caps.len, static_cast<uint16_t>(CAPLEN_RAIL));
    RED_CHECK_EQUAL(rail_caps.RailSupportLevel, static_cast<uint32_t>(1));

    StaticOutStream<1024> out_stream;
    rail_caps.emit(out_stream);

    InStream stream(out_stream.get_data(), out_stream.get_offset());

    RailCaps rail_caps2;

    RED_CHECK_EQUAL(rail_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_RAIL));
    RED_CHECK_EQUAL(rail_caps2.len, static_cast<uint16_t>(CAPLEN_RAIL));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_RAIL), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_RAIL), stream.in_uint16_le());
    rail_caps2.recv(stream, CAPLEN_RAIL);

    RED_CHECK_EQUAL(rail_caps2.RailSupportLevel, static_cast<uint32_t>(1));
}
