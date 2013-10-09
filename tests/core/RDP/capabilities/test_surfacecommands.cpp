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

   Unit test to RDP SurfaceCommands object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilitySurfaceCommandsEmit)
{
    SurfaceCommandsCaps surfacecommands_caps;
    surfacecommands_caps.cmdFlags = 65536;
    surfacecommands_caps.reserved = 65536;

    BOOST_CHECK_EQUAL(surfacecommands_caps.capabilityType, static_cast<uint16_t>(CAPSETTYPE_SURFACE_COMMANDS));
    BOOST_CHECK_EQUAL(surfacecommands_caps.len, static_cast<uint16_t>(CAPLEN_SURFACE_COMMANDS));
    BOOST_CHECK_EQUAL(surfacecommands_caps.cmdFlags, (uint32_t) 65536);
    BOOST_CHECK_EQUAL(surfacecommands_caps.reserved, (uint32_t) 65536);

    BStream stream(1024);
    surfacecommands_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.get_data();

    SurfaceCommandsCaps surfacecommands_caps2;

    BOOST_CHECK_EQUAL(surfacecommands_caps2.capabilityType, static_cast<uint16_t>(CAPSETTYPE_SURFACE_COMMANDS));
    BOOST_CHECK_EQUAL(surfacecommands_caps2.len, static_cast<uint16_t>(CAPLEN_SURFACE_COMMANDS));

    BOOST_CHECK_EQUAL((uint16_t)CAPSETTYPE_SURFACE_COMMANDS, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_SURFACE_COMMANDS, stream.in_uint16_le());
    surfacecommands_caps2.recv(stream, CAPLEN_SURFACE_COMMANDS);

    BOOST_CHECK_EQUAL(surfacecommands_caps2.cmdFlags, (uint32_t) 65536);
    BOOST_CHECK_EQUAL(surfacecommands_caps2.reserved, (uint32_t) 65536);
}
