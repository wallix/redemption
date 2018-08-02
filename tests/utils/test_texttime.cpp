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
*1324
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2017
*   Author(s): Clément Moroldo, Christophe Grosjean
*/

#include "utils/log.hpp"

#define RED_TEST_MODULE TestTexttime
#include "system/redemption_unit_tests.hpp"

#include "utils/texttime.hpp"

#include <sys/time.h>

RED_AUTO_TEST_CASE(TestTextdate)
{
    BOOST_CHECK_EQUAL(std::string("1970-01-01"), text_gmdate(0));

    BOOST_CHECK_EQUAL(std::string("1970 01 01"), text_gmdate(0, ' '));

//     struct timeval tv;
//     struct timezone tz;
//     gettimeofday(&tv,&tz);
//     LOG(LOG_INFO, "tv.sec = %d", tv.tv_sec);
    BOOST_CHECK_EQUAL(std::string("2018-08-02"), text_gmdate(1533211681));

    BOOST_CHECK_EQUAL(std::string("1970-01-01 00:00:00"), text_gmdatetime(0));

    BOOST_CHECK_EQUAL(std::string("1970-01-01_00-00-00"), filename_gmdatetime(0));

}
