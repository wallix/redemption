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

   Unit test to RDP FrameAcknowledge object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityFrameAcknowledgeEmit)
{
    FrameAcknowledgeCaps frameacknowledge_caps;
    frameacknowledge_caps.maxUnacknowledgedFrameCount = 65536;

    BOOST_CHECK_EQUAL(frameacknowledge_caps.capabilityType, (uint16_t)CAPSETTYPE_FRAME_ACKNOWLEDGE);
    BOOST_CHECK_EQUAL(frameacknowledge_caps.len, (uint16_t)CAPLEN_FRAME_ACKNOWLEDGE);
    BOOST_CHECK_EQUAL(frameacknowledge_caps.maxUnacknowledgedFrameCount, (uint32_t) 65536);

    BStream stream(1024);
    frameacknowledge_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.data;

    FrameAcknowledgeCaps frameacknowledge_caps2;

    BOOST_CHECK_EQUAL(frameacknowledge_caps2.capabilityType, (uint16_t)CAPSETTYPE_FRAME_ACKNOWLEDGE);
    BOOST_CHECK_EQUAL(frameacknowledge_caps2.len, (uint16_t)CAPLEN_FRAME_ACKNOWLEDGE);

    BOOST_CHECK_EQUAL((uint16_t)CAPSETTYPE_FRAME_ACKNOWLEDGE, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_FRAME_ACKNOWLEDGE, stream.in_uint16_le());
    frameacknowledge_caps2.recv(stream, CAPLEN_FRAME_ACKNOWLEDGE);

    BOOST_CHECK_EQUAL(frameacknowledge_caps2.maxUnacknowledgedFrameCount, (uint32_t) 65536);
}
