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

   Unit test to RDP MultiFragmentUpdate object
   Using lib boost functions for testing
*/
#define RED_TEST_MODULE CapabilityMultiFragmentUpdateEmit

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/capabilities/multifragmentupdate.hpp"

RED_AUTO_TEST_CASE(TestCapabilityMultiFragmentUpdateEmit)
{
    MultiFragmentUpdateCaps multifragmentupdate_caps;
    multifragmentupdate_caps.MaxRequestSize = 65536;

    RED_CHECK_EQUAL(multifragmentupdate_caps.capabilityType, static_cast<uint16_t>(CAPSETTYPE_MULTIFRAGMENTUPDATE));
    RED_CHECK_EQUAL(multifragmentupdate_caps.len, static_cast<uint16_t>(CAPLEN_MULTIFRAGMENTUPDATE));
    RED_CHECK_EQUAL(multifragmentupdate_caps.MaxRequestSize, static_cast<uint32_t>(65536));

    StaticOutStream<1024> out_stream;
    multifragmentupdate_caps.emit(out_stream);

    InStream stream(out_stream.get_bytes());

    MultiFragmentUpdateCaps multifragmentupdate_caps2;

    RED_CHECK_EQUAL(multifragmentupdate_caps2.capabilityType, static_cast<uint16_t>(CAPSETTYPE_MULTIFRAGMENTUPDATE));
    RED_CHECK_EQUAL(multifragmentupdate_caps2.len, static_cast<uint16_t>(CAPLEN_MULTIFRAGMENTUPDATE));

    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPSETTYPE_MULTIFRAGMENTUPDATE), stream.in_uint16_le());
    RED_CHECK_EQUAL(static_cast<uint16_t>(CAPLEN_MULTIFRAGMENTUPDATE), stream.in_uint16_le());
    multifragmentupdate_caps2.recv(stream, CAPLEN_MULTIFRAGMENTUPDATE);

    RED_CHECK_EQUAL(multifragmentupdate_caps2.MaxRequestSize, static_cast<uint32_t>(65536));
}
