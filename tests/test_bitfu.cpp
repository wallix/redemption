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

   Unit test for bitmap class, compression performance

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBitFu
#include <boost/test/auto_unit_test.hpp>

#include"log.hpp"
#define LOGNULL

#include"bitfu.hpp"
#include <stdio.h>

//#include"rdtsc.hpp"

//long long ustime() {
//    struct timeval now;
//    gettimeofday(&now, NULL);
//    return (long long)now.tv_sec*1000000LL + (long long)now.tv_usec;
//}

//BOOST_AUTO_TEST_CASE(TestReversePerf)
//{
//    unsigned long long usec = ustime();
//    unsigned long long cycles = rdtsc();
//    const size_t max = 10000;
//    uint32_t buf[max];
//    for (size_t i = 0; i < max; i++){
//        buf[i] = i;
//    }
//    for (size_t i = 0; i < 1000 ; i++){
//        reverseit((uint8_t*)buf, max*sizeof(uint32_t));
//    }
//    unsigned long long elapusec = ustime() - usec;
//    unsigned long long elapcyc = rdtsc() - cycles;
//    printf("elapsed time = %llu %llu %f\n", elapusec, elapcyc, (double)elapcyc / (double)elapusec);
//}

BOOST_AUTO_TEST_CASE(TestReverseEven)
{
    uint8_t buf[6] = {0, 1, 2, 3, 4, 5};
    reverseit(buf, 6);
    BOOST_CHECK_EQUAL(buf[0], 5);
    BOOST_CHECK_EQUAL(buf[1], 4);
    BOOST_CHECK_EQUAL(buf[2], 3);
    BOOST_CHECK_EQUAL(buf[3], 2);
    BOOST_CHECK_EQUAL(buf[4], 1);
    BOOST_CHECK_EQUAL(buf[5], 0);
}

BOOST_AUTO_TEST_CASE(TestReverseOdd)
{
    uint8_t buf[5] = {1, 2, 3, 4, 5};
    reverseit(buf, 5);
    BOOST_CHECK_EQUAL(buf[0], 5);
    BOOST_CHECK_EQUAL(buf[1], 4);
    BOOST_CHECK_EQUAL(buf[2], 3);
    BOOST_CHECK_EQUAL(buf[3], 2);
    BOOST_CHECK_EQUAL(buf[4], 1);
}

