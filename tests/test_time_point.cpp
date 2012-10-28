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
#define BOOST_TEST_MODULE TestTimePoint
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "time_point.hpp"


void time_point_error(const std::string& s)
{
    try {
        time_point t(s);
        std::string mess("time_point(\"");
        mess += s;
        mess += "\") pass";
        BOOST_CHECK_MESSAGE(false, mess);
    } catch (std::exception& ) {
    }
}

BOOST_AUTO_TEST_CASE(TestTimePointParser)
{
    BOOST_CHECK(time_point("").time == 0);
    BOOST_CHECK(time_point("1").time == 1);
    BOOST_CHECK(time_point("+1").time == 1);
    BOOST_CHECK(time_point("10").time == 10);
    BOOST_CHECK(time_point("10+8").time == 18);
    BOOST_CHECK(time_point("1h").time == 3600);
    BOOST_CHECK(time_point("1m").time == 60);
    BOOST_CHECK(time_point("1s").time == 1);
    BOOST_CHECK(time_point("1s+1m").time == 61);
    BOOST_CHECK(time_point("1h-1s").time == 3599);
    BOOST_CHECK(time_point("1h-1m").time == 3540);
    BOOST_CHECK(time_point("1h-1m-1s").time == 3539);
    BOOST_CHECK(time_point("1h+1m+1s").time == 3661);
    BOOST_CHECK(time_point("1h30").time == 3600+30*60);
    BOOST_CHECK(time_point("1h30m5s").time == 3600+30*60+5);
    BOOST_CHECK(time_point("5h458m9s").time == 5*3600+458*60+9);
    time_point_error("hm");
    time_point_error("1hm");
    time_point_error("+");
    time_point_error("-");
}
