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
   Copyright (C) Wallix 2011
   Author(s): Martin Potier

   Unit test to URT object
   URT = µsecond Redemption Time

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestURT
#include <boost/test/auto_unit_test.hpp>

#include "urt.hpp"

BOOST_AUTO_TEST_CASE(TestURT_test_instances)
{
    {
        timeval t;
        gettimeofday(&t, NULL);

        URT time(t);

        BOOST_CHECK_EQUAL(t.tv_sec, time.tv.tv_sec);
        BOOST_CHECK_EQUAL(t.tv_usec, time.tv.tv_usec);
    }

    {
        timeval t;
        gettimeofday(&t, NULL);

        URT time;

        BOOST_CHECK(t.tv_sec  <= time.tv.tv_sec);
        BOOST_CHECK(t.tv_usec <= time.tv.tv_usec);
    }

    {
        URT t(544544);

        BOOST_CHECK_EQUAL(t.tv.tv_sec, 0);
        BOOST_CHECK_EQUAL(t.tv.tv_usec, 544544);
    }

    {
        URT t(1000042);

        BOOST_CHECK_EQUAL(t.tv.tv_sec, 1);
        BOOST_CHECK_EQUAL(t.tv.tv_usec, 42);
    }
}

BOOST_AUTO_TEST_CASE(TestURT_test_equal)
{
    timeval t;
    gettimeofday(&t, NULL);

    URT time(t);
    URT time2(t);

    BOOST_CHECK(time == time2);
}

BOOST_AUTO_TEST_CASE(TestURT_test_not_equal)
{
    URT time1;
    URT time2;

    time2.tv.tv_usec += 1;

    BOOST_CHECK(time1 != time2);
}

BOOST_AUTO_TEST_CASE(TestURT_test_addition)
{
    timeval t;
    gettimeofday(&t, NULL);

    URT time1;
    URT time2;
    URT time3;

    time1.tv.tv_sec = 1; time1.tv.tv_usec = 999997;
    time2.tv.tv_sec = 1; time2.tv.tv_usec = 2;

    time3 = time1 + time2;

    BOOST_CHECK(time3.tv.tv_sec == 2);
    BOOST_CHECK(time3.tv.tv_usec == 999999);

    time2.tv.tv_sec = 1; time2.tv.tv_usec = 4;

    time3 = time1 + time2;

    BOOST_CHECK(time3.tv.tv_sec == 3);
    BOOST_CHECK(time3.tv.tv_usec == 1);
}

BOOST_AUTO_TEST_CASE(TestURT_test_greater)
{
    timeval t;
    gettimeofday(&t, NULL);

    URT time1(t);
    URT time2(t);

    BOOST_CHECK(time2 >= time1);

    time2.tv.tv_sec = 2;
    time1.tv.tv_sec = 1;

    BOOST_CHECK(time2 > time1);

    time2.tv.tv_sec = 1; time2.tv.tv_usec = 2;
    time1.tv.tv_sec = 1; time1.tv.tv_usec = 1;

    BOOST_CHECK(time2 > time1);
}

BOOST_AUTO_TEST_CASE(TestURT_test_lower)
{
    timeval t;
    gettimeofday(&t, NULL);

    URT time1(t);
    URT time2(t);

    BOOST_CHECK(time2 <= time1);

    time2.tv.tv_sec = 2;
    time1.tv.tv_sec = 1;

    BOOST_CHECK(time1 < time2);

    time2.tv.tv_sec = 1; time2.tv.tv_usec = 2;
    time1.tv.tv_sec = 1; time1.tv.tv_usec = 1;

    BOOST_CHECK(time1 < time2);
}
