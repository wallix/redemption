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

   Unit test to RDP Activate object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityActivation
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityActivationEmit)
{
    ActivationCaps activation_caps;
    activation_caps.helpKeyFlag = 0;
    activation_caps.helpKeyIndexFlag = 1;
    activation_caps.helpExtendedKeyFlag = 2;
    activation_caps.windowManagerKeyFlag = 3;

    BOOST_CHECK_EQUAL(activation_caps.capabilityType, (uint16_t)CAPSTYPE_ACTIVATION);
    BOOST_CHECK_EQUAL(activation_caps.len, (uint16_t)CAPLEN_ACTIVATION);
    BOOST_CHECK_EQUAL(activation_caps.helpKeyFlag, (uint16_t) 0);
    BOOST_CHECK_EQUAL(activation_caps.helpKeyIndexFlag, (uint16_t) 1);
    BOOST_CHECK_EQUAL(activation_caps.helpExtendedKeyFlag, (uint16_t) 2);
    BOOST_CHECK_EQUAL(activation_caps.windowManagerKeyFlag, (uint16_t) 3);

    Stream stream(1024);
    activation_caps.emit(stream);
    stream.end = stream.p;
    stream.p = stream.data;

    ActivationCaps activation_caps2;

    BOOST_CHECK_EQUAL(activation_caps2.capabilityType, (uint16_t)CAPSTYPE_ACTIVATION);
    BOOST_CHECK_EQUAL(activation_caps2.len, (uint16_t)CAPLEN_ACTIVATION);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_ACTIVATION, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_ACTIVATION, stream.in_uint16_le());
    activation_caps2.recv(stream, CAPLEN_ACTIVATION);

    BOOST_CHECK_EQUAL(activation_caps2.helpKeyFlag, (uint16_t) 0);
    BOOST_CHECK_EQUAL(activation_caps2.helpKeyIndexFlag, (uint16_t) 1);
    BOOST_CHECK_EQUAL(activation_caps2.helpExtendedKeyFlag, (uint16_t) 2);
    BOOST_CHECK_EQUAL(activation_caps2.windowManagerKeyFlag, (uint16_t) 3);
}
