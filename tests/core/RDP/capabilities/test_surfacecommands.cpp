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
#define RED_TEST_MODULE CapabilitySurfaceCommandsEmit

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/surfacecommands.hpp"

RED_AUTO_TEST_CASE(TestCapabilitySurfaceCommandsEmit)
{
    SurfaceCommandsCaps surfacecommands_caps;
    surfacecommands_caps.cmdFlags = 65536;
    surfacecommands_caps.reserved = 65536;

    RED_CHECK_EQUAL(surfacecommands_caps.capabilityType, static_cast<uint16_t>(CAPSETTYPE_SURFACE_COMMANDS));
    RED_CHECK_EQUAL(surfacecommands_caps.len, static_cast<uint16_t>(CAPLEN_SURFACE_COMMANDS));
    RED_CHECK_EQUAL(surfacecommands_caps.cmdFlags, static_cast<uint32_t>(65536));
    RED_CHECK_EQUAL(surfacecommands_caps.reserved, static_cast<uint32_t>(65536));

    StaticOutStream<1024> out_stream;
    surfacecommands_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    SurfaceCommandsCaps surfacecommands_caps2;

    RED_CHECK_EQUAL(surfacecommands_caps2.capabilityType, static_cast<uint16_t>(CAPSETTYPE_SURFACE_COMMANDS));
    RED_CHECK_EQUAL(surfacecommands_caps2.len, static_cast<uint16_t>(CAPLEN_SURFACE_COMMANDS));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSETTYPE_SURFACE_COMMANDS), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_SURFACE_COMMANDS), stream.in_uint16_le());
    surfacecommands_caps2.recv(stream, CAPLEN_SURFACE_COMMANDS);

    RED_CHECK_EQUAL(surfacecommands_caps2.cmdFlags, static_cast<uint32_t>(65536));
    RED_CHECK_EQUAL(surfacecommands_caps2.reserved, static_cast<uint32_t>(65536));
}
