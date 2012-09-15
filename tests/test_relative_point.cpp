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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRelativePoint
#include <boost/test/auto_unit_test.hpp>

#include "relative_time_point.hpp"


BOOST_AUTO_TEST_CASE(TestRelativeTimePoint)
{
    BOOST_CHECK(relative_time_point("").point.time == 0);
    BOOST_CHECK(relative_time_point("1").point.time == 1);
    BOOST_CHECK(relative_time_point("+1").point.time == 1);
    BOOST_CHECK(relative_time_point("10").point.time == 10);
    BOOST_CHECK(relative_time_point("10+8").point.time == 18);
    BOOST_CHECK(relative_time_point("1h").point.time == 3600);
    BOOST_CHECK(relative_time_point("1m").point.time == 60);
    BOOST_CHECK(relative_time_point("-1s").point.time == 1);
    BOOST_CHECK(relative_time_point("1s+1m").point.time == 61);
    BOOST_CHECK(relative_time_point("1h-1s").point.time == 3599);
    BOOST_CHECK(relative_time_point("-1h-1m").point.time == 3540);
    BOOST_CHECK(relative_time_point("1h-1m-1s").point.time == 3539);
    BOOST_CHECK(relative_time_point("-1h+1m+1s").point.time == 3661);
    BOOST_CHECK(relative_time_point("-1h30").point.time == 3600+30*60);
    BOOST_CHECK(relative_time_point("+1h30m5s").point.time == 3600+30*60+5);
    BOOST_CHECK(relative_time_point("+5h458m9s").point.time == 5*3600+458*60+9);
}

BOOST_AUTO_TEST_CASE(TestRelativeRangeTimePoint)
{
    std::size_t max = std::numeric_limits<std::size_t>::max();

    BOOST_CHECK(relative_range_time_point("").point == range_time_point(0,max));
    BOOST_CHECK(relative_range_time_point(",").point == range_time_point(0,max));
    BOOST_CHECK(relative_range_time_point("1").point == range_time_point(0,1));
    BOOST_CHECK(relative_range_time_point("1,").point == range_time_point(1,max));
    BOOST_CHECK(relative_range_time_point("+,1").point == range_time_point(0,1));
    BOOST_CHECK(relative_range_time_point("10").point == range_time_point(0,10));
    BOOST_CHECK(relative_range_time_point("10,").point == range_time_point(10,max));
    BOOST_CHECK(relative_range_time_point("+,10").point == range_time_point(0,10));
    BOOST_CHECK(relative_range_time_point("-,10+8").point == range_time_point(0,18));
    BOOST_CHECK(relative_range_time_point("+10+8,").point == range_time_point(18,max));
    BOOST_CHECK(relative_range_time_point("10,28").point == range_time_point(10,28));
    BOOST_CHECK(relative_range_time_point("+1,+5").point == range_time_point(1,6));
    BOOST_CHECK(relative_range_time_point("-1,5").point == range_time_point(1,5));
}
