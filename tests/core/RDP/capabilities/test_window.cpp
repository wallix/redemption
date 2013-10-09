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

   Unit test to RDP Rail object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilityWindowsListEmit)
{
    WindowsListCaps windowslist_caps;
    windowslist_caps.WndSupportLevel = TS_WINDOW_LEVEL_SUPPORTED_EX;
    windowslist_caps.NumIconCaches = 255;
    windowslist_caps.NumIconCacheEntries = 65535;

    BOOST_CHECK_EQUAL(windowslist_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_WINDOW));
    BOOST_CHECK_EQUAL(windowslist_caps.len, static_cast<uint16_t>(CAPLEN_WINDOW));
    BOOST_CHECK_EQUAL(windowslist_caps.WndSupportLevel, static_cast<uint32_t>(2));
    BOOST_CHECK_EQUAL(windowslist_caps.NumIconCaches, static_cast<uint8_t>(255));
    BOOST_CHECK_EQUAL(windowslist_caps.NumIconCacheEntries, static_cast<uint16_t>(65535));

    BStream stream(1024);
    windowslist_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.get_data();

    WindowsListCaps windowslist_caps2;

    BOOST_CHECK_EQUAL(windowslist_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_WINDOW));
    BOOST_CHECK_EQUAL(windowslist_caps2.len, static_cast<uint16_t>(CAPLEN_WINDOW));

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_WINDOW, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_WINDOW, stream.in_uint16_le());
    windowslist_caps2.recv(stream, CAPLEN_WINDOW);

    BOOST_CHECK_EQUAL(windowslist_caps2.WndSupportLevel, static_cast<uint32_t>(2));
    BOOST_CHECK_EQUAL(windowslist_caps2.NumIconCaches, static_cast<uint8_t>(255));
    BOOST_CHECK_EQUAL(windowslist_caps2.NumIconCacheEntries, static_cast<uint16_t>(65535));
}
