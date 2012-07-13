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

   Unit test to RDP BmpCache object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityBmpCache
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityBmpCachepEmit)
{
    BmpCacheCaps bmpcache_caps;
    bmpcache_caps.pad1 = 1;
    bmpcache_caps.pad2 = 2;
    bmpcache_caps.pad3 = 3;
    bmpcache_caps.pad4 = 4;
    bmpcache_caps.pad5 = 5;
    bmpcache_caps.pad6 = 6;
    bmpcache_caps.cache0Entries = 200;
    bmpcache_caps.cache0MaximumCellSize = 32769;
    bmpcache_caps.cache1Entries = 600;
    bmpcache_caps.cache1MaximumCellSize = 32769;
    bmpcache_caps.cache2Entries = 65535;
    bmpcache_caps.cache2MaximumCellSize = 32769;

    BOOST_CHECK_EQUAL(bmpcache_caps.capabilityType, (uint16_t)CAPSTYPE_BITMAPCACHE);
    BOOST_CHECK_EQUAL(bmpcache_caps.len, (uint16_t)CAPLEN_BITMAPCACHE);
    BOOST_CHECK_EQUAL(bmpcache_caps.pad1, (uint32_t) 1);
    BOOST_CHECK_EQUAL(bmpcache_caps.pad2, (uint32_t) 2);
    BOOST_CHECK_EQUAL(bmpcache_caps.pad3, (uint32_t) 3);
    BOOST_CHECK_EQUAL(bmpcache_caps.pad4, (uint32_t) 4);
    BOOST_CHECK_EQUAL(bmpcache_caps.pad5, (uint32_t) 5);
    BOOST_CHECK_EQUAL(bmpcache_caps.pad6, (uint32_t) 6);
    BOOST_CHECK_EQUAL(bmpcache_caps.cache0Entries, (uint16_t) 200);
    BOOST_CHECK_EQUAL(bmpcache_caps.cache0MaximumCellSize, (uint16_t) 32769);
    BOOST_CHECK_EQUAL(bmpcache_caps.cache1Entries, (uint16_t) 600);
    BOOST_CHECK_EQUAL(bmpcache_caps.cache1MaximumCellSize, (uint16_t) 32769);
    BOOST_CHECK_EQUAL(bmpcache_caps.cache2Entries, (uint16_t) 65535);
    BOOST_CHECK_EQUAL(bmpcache_caps.cache2MaximumCellSize, (uint16_t) 32769);

    BStream stream(1024);
    bmpcache_caps.emit(stream);
    stream.end = stream.p;
    stream.p = stream.data;

    BmpCacheCaps bmpcache_caps2;

    BOOST_CHECK_EQUAL(bmpcache_caps2.capabilityType, (uint16_t)CAPSTYPE_BITMAPCACHE);
    BOOST_CHECK_EQUAL(bmpcache_caps2.len, (uint16_t)CAPLEN_BITMAPCACHE);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_BITMAPCACHE, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_BITMAPCACHE, stream.in_uint16_le());
    bmpcache_caps2.recv(stream, CAPLEN_BITMAPCACHE);

    BOOST_CHECK_EQUAL(bmpcache_caps2.pad1, (uint32_t) 1);
    BOOST_CHECK_EQUAL(bmpcache_caps2.pad2, (uint32_t) 2);
    BOOST_CHECK_EQUAL(bmpcache_caps2.pad3, (uint32_t) 3);
    BOOST_CHECK_EQUAL(bmpcache_caps2.pad4, (uint32_t) 4);
    BOOST_CHECK_EQUAL(bmpcache_caps2.pad5, (uint32_t) 5);
    BOOST_CHECK_EQUAL(bmpcache_caps2.pad6, (uint32_t) 6);
    BOOST_CHECK_EQUAL(bmpcache_caps2.cache0Entries, (uint16_t) 200);
    BOOST_CHECK_EQUAL(bmpcache_caps2.cache0MaximumCellSize, (uint16_t) 32769);
    BOOST_CHECK_EQUAL(bmpcache_caps2.cache1Entries, (uint16_t) 600);
    BOOST_CHECK_EQUAL(bmpcache_caps2.cache1MaximumCellSize, (uint16_t) 32769);
    BOOST_CHECK_EQUAL(bmpcache_caps2.cache2Entries, (uint16_t) 65535);
    BOOST_CHECK_EQUAL(bmpcache_caps2.cache2MaximumCellSize, (uint16_t) 32769);
}
