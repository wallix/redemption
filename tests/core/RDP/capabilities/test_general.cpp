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

   Unit test to RDP General object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityGeneral
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/general.hpp"

RED_AUTO_TEST_CASE(TestCapabilityGeneralEmit)
{
    GeneralCaps general_caps;
    general_caps.os_major = 0;
    general_caps.os_minor = 1;
    general_caps.protocolVersion = 2;
    general_caps.pad2octetsA = 3;
    general_caps.compressionType = 4;
    general_caps.extraflags = 65535;
    general_caps.updateCapability = 6;
    general_caps.remoteUnshare = 7;
    general_caps.compressionLevel = 8;
    general_caps.refreshRectSupport = 1;
    general_caps.suppressOutputSupport = 1;

    RED_CHECK_EQUAL(general_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_GENERAL));
    RED_CHECK_EQUAL(general_caps.len, static_cast<uint16_t>(CAPLEN_GENERAL));
    RED_CHECK_EQUAL(general_caps.os_major, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(general_caps.os_minor, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(general_caps.protocolVersion, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(general_caps.pad2octetsA, static_cast<uint16_t>(3));
    RED_CHECK_EQUAL(general_caps.compressionType, static_cast<uint16_t>(4));
    RED_CHECK_EQUAL(general_caps.extraflags, static_cast<uint16_t>(65535));
    RED_CHECK_EQUAL(general_caps.updateCapability, static_cast<uint16_t>(6));
    RED_CHECK_EQUAL(general_caps.remoteUnshare, static_cast<uint16_t>(7));
    RED_CHECK_EQUAL(general_caps.compressionLevel, static_cast<uint16_t>(8));
    RED_CHECK_EQUAL(general_caps.refreshRectSupport, static_cast<uint8_t>(1));
    RED_CHECK_EQUAL(general_caps.suppressOutputSupport, static_cast<uint8_t>(1));

    StaticOutStream<1024> out_stream;
    general_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    GeneralCaps general_caps2;

    RED_CHECK_EQUAL(general_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_GENERAL));
    RED_CHECK_EQUAL(general_caps2.len, static_cast<uint16_t>(CAPLEN_GENERAL));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_GENERAL), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_GENERAL), stream.in_uint16_le());

    general_caps2.recv(stream, CAPLEN_GENERAL);

    RED_CHECK_EQUAL(general_caps2.os_major, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(general_caps2.os_minor, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(general_caps2.protocolVersion, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(general_caps2.pad2octetsA, static_cast<uint16_t>(3));
    RED_CHECK_EQUAL(general_caps2.compressionType, static_cast<uint16_t>(4));
    RED_CHECK_EQUAL(general_caps2.extraflags, static_cast<uint16_t>(65535));
    RED_CHECK_EQUAL(general_caps2.updateCapability, static_cast<uint16_t>(6));
    RED_CHECK_EQUAL(general_caps2.remoteUnshare, static_cast<uint16_t>(7));
    RED_CHECK_EQUAL(general_caps2.compressionLevel, static_cast<uint16_t>(8));
    RED_CHECK_EQUAL(general_caps2.refreshRectSupport, static_cast<uint8_t>(1));
    RED_CHECK_EQUAL(general_caps2.suppressOutputSupport, static_cast<uint8_t>(1));
}
