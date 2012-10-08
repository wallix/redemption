/*
T his program is free software; you ca*n redistribute it and/or modify
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
Copyright (C) Wallix 2010-2012
Author(s): Christophe Grosjean, Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestTimerCapture
#include <boost/test/auto_unit_test.hpp>

#include "timer_capture.hpp"


BOOST_AUTO_TEST_CASE(TestTimerCapture)
{
    timeval now = {1000, 0};
    TimerCapture timer(now);
    BOOST_CHECK_EQUAL(timer.sec() != 0, true);
    uint64_t elapsed = difftimeval(now, timer.tv);
    timer.tv = now;
    BOOST_CHECK_EQUAL(elapsed, 0);

    now.tv_sec += 4;
    elapsed = difftimeval(now, timer.tv);
    timer.tv = now;
    BOOST_CHECK_EQUAL(elapsed, 4 * 1000000);

    now.tv_usec += 80;
    elapsed = difftimeval(now, timer.tv);
    timer.tv = now;
    BOOST_CHECK_EQUAL(elapsed, 80);

    now.tv_usec += 80;
    now.tv_sec += 1;
    elapsed = difftimeval(now, timer.tv);
    timer.tv = now;
    BOOST_CHECK_EQUAL(elapsed, 1 * 1000000 + 80);

    if (difftimeval(now, timer.tv) > 10){
        timer.tv = now;
        BOOST_CHECK(false);
    }

    now.tv_usec += 80;
    if (difftimeval(now, timer.tv) <= 10){
        BOOST_CHECK(false);
    }
    timer.tv = now;
    elapsed = difftimeval(now, timer.tv);
    timer.tv = now;
    BOOST_CHECK_EQUAL(elapsed, 0);
}
