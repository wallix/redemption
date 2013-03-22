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

   Unit test to RDP General object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityGeneral
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include"log.hpp"
#include"RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityGeneralEmit)
{
    GeneralCaps general_caps;
    general_caps.os_major = 0;
    general_caps.os_minor = 1;
    general_caps.protocolVersion = 2;
    general_caps.pad1 = 3;
    general_caps.compressionType = 4;
    general_caps.extraflags = 65535;
    general_caps.updateCapability = 6;
    general_caps.remoteUnshare = 7;
    general_caps.compressionLevel = 8;
    general_caps.pad2 = 9;

    BOOST_CHECK_EQUAL(general_caps.capabilityType, (uint16_t)CAPSTYPE_GENERAL);
    BOOST_CHECK_EQUAL(general_caps.len, (uint16_t)CAPLEN_GENERAL);
    BOOST_CHECK_EQUAL(general_caps.os_major, (uint16_t) 0);
    BOOST_CHECK_EQUAL(general_caps.os_minor, (uint16_t) 1);
    BOOST_CHECK_EQUAL(general_caps.protocolVersion, (uint16_t) 2);
    BOOST_CHECK_EQUAL(general_caps.pad1, (uint16_t) 3);
    BOOST_CHECK_EQUAL(general_caps.compressionType, (uint16_t) 4);
    BOOST_CHECK_EQUAL(general_caps.extraflags, (uint16_t) 65535);
    BOOST_CHECK_EQUAL(general_caps.updateCapability, (uint16_t) 6);
    BOOST_CHECK_EQUAL(general_caps.remoteUnshare, (uint16_t) 7);
    BOOST_CHECK_EQUAL(general_caps.compressionLevel, (uint16_t) 8);
    BOOST_CHECK_EQUAL(general_caps.pad2, (uint16_t) 9);

    BStream stream(1024);
    general_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.data;

    GeneralCaps general_caps2;

    BOOST_CHECK_EQUAL(general_caps2.capabilityType, (uint16_t)CAPSTYPE_GENERAL);
    BOOST_CHECK_EQUAL(general_caps2.len, (uint16_t)CAPLEN_GENERAL);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_GENERAL, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_GENERAL, stream.in_uint16_le());

    general_caps2.recv(stream, CAPLEN_GENERAL);

    BOOST_CHECK_EQUAL(general_caps2.os_major, (uint16_t) 0);
    BOOST_CHECK_EQUAL(general_caps2.os_minor, (uint16_t) 1);
    BOOST_CHECK_EQUAL(general_caps2.protocolVersion, (uint16_t) 2);
    BOOST_CHECK_EQUAL(general_caps2.pad1, (uint16_t) 3);
    BOOST_CHECK_EQUAL(general_caps2.compressionType, (uint16_t) 4);
    BOOST_CHECK_EQUAL(general_caps2.extraflags, (uint16_t) 65535);
    BOOST_CHECK_EQUAL(general_caps2.updateCapability, (uint16_t) 6);
    BOOST_CHECK_EQUAL(general_caps2.remoteUnshare, (uint16_t) 7);
    BOOST_CHECK_EQUAL(general_caps2.compressionLevel, (uint16_t) 8);
    BOOST_CHECK_EQUAL(general_caps2.pad2, (uint16_t) 9);
}
