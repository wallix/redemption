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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to test back and forth conversion of aligned to compact bitmaps

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestALignedToCompact
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include "bitfu.hpp"


BOOST_AUTO_TEST_CASE(TestCompactToAlignedSizeComputing)
{
    BOOST_CHECK_EQUAL(800, row_size(800, 8));
    BOOST_CHECK_EQUAL(1600, row_size(800, 16));
    BOOST_CHECK_EQUAL(2400, row_size(800, 24));
    BOOST_CHECK_EQUAL(3200, row_size(800, 32));

    BOOST_CHECK_EQUAL(800, row_size(799, 8));
    BOOST_CHECK_EQUAL(1600, row_size(799, 16));
    BOOST_CHECK_EQUAL(2400, row_size(799, 24));
    BOOST_CHECK_EQUAL(3196, row_size(799, 32));

    BOOST_CHECK_EQUAL(800, row_size(798, 8));
    BOOST_CHECK_EQUAL(1596, row_size(798, 16));
    BOOST_CHECK_EQUAL(2396, row_size(798, 24));
    BOOST_CHECK_EQUAL(3192, row_size(798, 32));

    BOOST_CHECK_EQUAL(800, row_size(797, 8));
    BOOST_CHECK_EQUAL(1596, row_size(797, 16));
    BOOST_CHECK_EQUAL(2392, row_size(797, 24));
    BOOST_CHECK_EQUAL(3188, row_size(797, 32));

    BOOST_CHECK_EQUAL(796, row_size(796, 8));
    BOOST_CHECK_EQUAL(1592, row_size(796, 16));
    BOOST_CHECK_EQUAL(2388, row_size(796, 24));
    BOOST_CHECK_EQUAL(3184, row_size(796, 32));
}

