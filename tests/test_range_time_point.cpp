/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 *   Based on xrdp Copyright (C) Jay Sorg 2004-2010
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRangeTimePoint
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "range_time_point.hpp"

void range_time_point_error(const std::string& s)
{
    try {
        range_time_point r(s);
        std::string mess("range_time_point(\"");
        mess += s;
        mess += "\") pass";
        BOOST_CHECK_MESSAGE(false, mess);
    } catch (std::exception& ) {
    }
}

BOOST_AUTO_TEST_CASE(TestRangeTimePointParser)
{
    std::size_t max = std::numeric_limits<std::size_t>::max();

    BOOST_CHECK(range_time_point("") == range_time_point(0,max));
    BOOST_CHECK(range_time_point(",") == range_time_point(0,max));
    BOOST_CHECK(range_time_point("1") == range_time_point(0,1));
    BOOST_CHECK(range_time_point("1,") == range_time_point(1,max));
    BOOST_CHECK(range_time_point(",1") == range_time_point(0,1));
    BOOST_CHECK(range_time_point("10") == range_time_point(0,10));
    BOOST_CHECK(range_time_point("10,") == range_time_point(10,max));
    BOOST_CHECK(range_time_point(",10") == range_time_point(0,10));
    BOOST_CHECK(range_time_point(",10+8") == range_time_point(0,18));
    BOOST_CHECK(range_time_point("+10+8,") == range_time_point(18,max));
    BOOST_CHECK(range_time_point("10,28") == range_time_point(10,28));
    range_time_point_error("10,28,");
    range_time_point_error("+,");
    range_time_point_error(",+");
    range_time_point_error("+,+");
    range_time_point_error("-");
    BOOST_CHECK(range_time_point("1,+5") == range_time_point(1,6));
    BOOST_CHECK(range_time_point("-1,5") == range_time_point(4,5));
    BOOST_CHECK(range_time_point("-1,+5") == range_time_point(4,9));
    //NOTE result is undetermined for range_time_point("-1,+-5");
}
