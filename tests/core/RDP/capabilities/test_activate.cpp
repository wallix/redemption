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

   Unit test to RDP Activate object
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestCapabilityActivation
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/capabilities/activate.hpp"

RED_AUTO_TEST_CASE(TestCapabilityActivationEmit)
{
    ActivationCaps activation_caps;
    activation_caps.helpKeyFlag = 0;
    activation_caps.helpKeyIndexFlag = 1;
    activation_caps.helpExtendedKeyFlag = 2;
    activation_caps.windowManagerKeyFlag = 3;

    RED_CHECK_EQUAL(activation_caps.capabilityType, CAPSTYPE_ACTIVATION);
    RED_CHECK_EQUAL(activation_caps.len, static_cast<uint16_t>(CAPLEN_ACTIVATION));
    RED_CHECK_EQUAL(activation_caps.helpKeyFlag, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(activation_caps.helpKeyIndexFlag, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(activation_caps.helpExtendedKeyFlag, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(activation_caps.windowManagerKeyFlag, static_cast<uint16_t>(3));

    StaticOutStream<1024> out_stream;
    activation_caps.emit(out_stream);
    InStream stream(out_stream.get_bytes());

    ActivationCaps activation_caps2;

    RED_CHECK_EQUAL(activation_caps2.capabilityType, CAPSTYPE_ACTIVATION);
    RED_CHECK_EQUAL(activation_caps2.len, static_cast<uint16_t>(CAPLEN_ACTIVATION));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSTYPE_ACTIVATION), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_ACTIVATION), stream.in_uint16_le());
    activation_caps2.recv(stream, CAPLEN_ACTIVATION);

    RED_CHECK_EQUAL(activation_caps2.helpKeyFlag, static_cast<uint16_t>(0));
    RED_CHECK_EQUAL(activation_caps2.helpKeyIndexFlag, static_cast<uint16_t>(1));
    RED_CHECK_EQUAL(activation_caps2.helpExtendedKeyFlag, static_cast<uint16_t>(2));
    RED_CHECK_EQUAL(activation_caps2.windowManagerKeyFlag, static_cast<uint16_t>(3));
}
