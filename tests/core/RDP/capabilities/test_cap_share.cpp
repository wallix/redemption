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

   Unit test to RDP Share object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityShare
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/capabilities/cap_share.hpp"

RED_AUTO_TEST_CASE(TestCapabilityShareEmit)
{
    ShareCaps share_caps;
    share_caps.nodeId = 0;
    share_caps.pad2octets = 1;

    RED_CHECK_EQUAL(share_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_SHARE));
    RED_CHECK_EQUAL(share_caps.len, static_cast<uint16_t>(CAPLEN_SHARE));
    RED_CHECK_EQUAL(share_caps.nodeId, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(share_caps.pad2octets, static_cast<uint16_t>(1));

    StaticOutStream<1024> out_stream;
    share_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    ShareCaps share_caps2;

    RED_CHECK_EQUAL(share_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_SHARE));
    RED_CHECK_EQUAL(share_caps2.len, static_cast<uint16_t>(CAPLEN_SHARE));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_SHARE), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_SHARE), stream.in_uint16_le());
    share_caps2.recv(stream, CAPLEN_SHARE);

    RED_CHECK_EQUAL(share_caps2.nodeId, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(share_caps2.pad2octets, static_cast<uint16_t>(1));
}
