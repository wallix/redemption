/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestTestRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "../libs/rio.h"
#include "../libs/rio_impl.h"


// TestTransport is basically (and internally) a Generator comined with a Check Transport
// It is designed fot testing purpose of bidirectional transports
// ie:
// - for testing an input file behavior, we will use a Generator
// - for testing an output file behavior, we wll use a Check
// - for testing a socket performing alternated sending and receiving, 
// we use Test Transport
// (obviously a Test transport without input data will behave like Check,
// and a Test Transport without output data will behave like Generator)

BOOST_AUTO_TEST_CASE(TestTestRIO)
{
    // Test Transport behave as a Check when we perform only send
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_test(&status, "output", 6, "input", 5);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rio_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rio_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rio_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_send(rt, "xxx", 3));

    rio_delete(rt);
}

BOOST_AUTO_TEST_CASE(TestTestRIO2)
{
    // Test Transport behave as a generator when we perform only receives
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_test(&status, "output", 6, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rio_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rio_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer+24, 1024));
    
    rio_delete(rt);
}


// we can also alternate send and recv
BOOST_AUTO_TEST_CASE(TestTestRIO3)
{
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_test(&status, "output", 6, "input", 5);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    uint8_t buffer[1024];

    BOOST_CHECK_EQUAL(3, rio_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("inp", buffer, 3));
    
    BOOST_CHECK_EQUAL(2, rio_send(rt, "ou", 2));
    BOOST_CHECK_EQUAL(4, rio_send(rt, "tput", 4));

    BOOST_CHECK_EQUAL(2, rio_recv(rt, buffer, 2));
    BOOST_CHECK_EQUAL(0, memcmp("ut", buffer, 2));

    BOOST_CHECK_EQUAL(0, rio_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_send(rt, "xxx", 3));

    // Now subsequent calls to send or recv always return the same error
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_recv(rt, buffer, 3));

    rio_delete(rt);
}

// Same as above but without heap memory allocation for RIO object
// (internally there is still memory allocated on heap for data buffers
// it is freed when calling clear)
BOOST_AUTO_TEST_CASE(TestTestRIO4)
{
    RIO rt; 
    RIO_ERROR status = rio_init_test(&rt, "output", 6, "input", 5);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    uint8_t buffer[1024];

    BOOST_CHECK_EQUAL(3, rio_recv(&rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("inp", buffer, 3));
    
    BOOST_CHECK_EQUAL(2, rio_send(&rt, "ou", 2));
    BOOST_CHECK_EQUAL(4, rio_send(&rt, "tput", 4));

    BOOST_CHECK_EQUAL(2, rio_recv(&rt, buffer, 2));
    BOOST_CHECK_EQUAL(0, memcmp("ut", buffer, 2));

    BOOST_CHECK_EQUAL(0, rio_send(&rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_send(&rt, "xxx", 3));

    // Now subsequent calls to send or recv always return the same error
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_recv(&rt, buffer, 3));

    rio_clear(&rt);
}

