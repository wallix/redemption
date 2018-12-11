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

   Unit test to RDP Control object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityControl
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/capabilities/control.hpp"

RED_AUTO_TEST_CASE(TestCapabilityControlEmit)
{
    ControlCaps control_caps;
    control_caps.controlFlags = 0;
    control_caps.remoteDetachFlag = 1;
    control_caps.controlInterest = 2;
    control_caps.detachInterest = 3;

    RED_CHECK_EQUAL(control_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_CONTROL));
    RED_CHECK_EQUAL(control_caps.len, static_cast<uint16_t>(CAPLEN_CONTROL));
    RED_CHECK_EQUAL(control_caps.controlFlags, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(control_caps.remoteDetachFlag, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(control_caps.controlInterest, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(control_caps.detachInterest, static_cast<uint16_t>(3));

    StaticOutStream<1024> out_stream;
    control_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    ControlCaps control_caps2;

    RED_CHECK_EQUAL(control_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_CONTROL));
    RED_CHECK_EQUAL(control_caps2.len, static_cast<uint16_t>(CAPLEN_CONTROL));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_CONTROL), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_CONTROL), stream.in_uint16_le());

    control_caps2.recv(stream, CAPLEN_CONTROL);

    RED_CHECK_EQUAL(control_caps2.controlFlags, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(control_caps2.remoteDetachFlag, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(control_caps2.controlInterest, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(control_caps2.detachInterest, static_cast<uint16_t>(3));
}
