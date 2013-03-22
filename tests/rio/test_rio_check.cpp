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
#define BOOST_TEST_MODULE TestCheckRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include"log.hpp"

#include"rio/rio.h"
#include"rio/rio_impl.h"

BOOST_AUTO_TEST_CASE(TestCheck)
{
    // RIO Check is used in test to verify data writtent from test body
    // instead of reading it from system files
    // It helps writing tests running faster and easier to read

    // Check Transport it somewhat similar to Generator Transport,
    // but instead of allowing to read what is provided
    // it checks that was is written to it is the provided reference string

    // We initialize it with some buffer data and size
    // When Check consumers will call "rio_send" the data will be compared with Check data

    // Mismatching data are shown in logs 
    // (logs appearing when this test is executed are normal)

    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_check(&status, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    // If both strings match, return length of sent buffer
    BOOST_CHECK_EQUAL(7, rio_send(rt, "We read", 7));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(8, rio_send(rt, " what we!!!", 11));
    // Now the next call will mismatch
    BOOST_CHECK_EQUAL(-RIO_ERROR_DATA_MISMATCH, rio_send(rt, "xxx", 3));

    rio_delete(rt);
}

// Same as above but this test calls init/clear instead of new/delete
BOOST_AUTO_TEST_CASE(TestCheck2)
{
    RIO rt;
    RIO_ERROR status = rio_init_check(&rt, "We read what we provide!", 24);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    BOOST_CHECK_EQUAL(7, rio_send(&rt, "We read", 7));
    BOOST_CHECK_EQUAL(8, rio_send(&rt, " what we!!!", 11));
    BOOST_CHECK_EQUAL(-RIO_ERROR_DATA_MISMATCH, rio_send(&rt, "xxx", 3));

    rio_clear(&rt);
}

// This test check behavior (error returned) when we send expected data
// but we continue sending data as check consumed all it's available data
BOOST_AUTO_TEST_CASE(TestCheckTransport2)
{
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_check(&status, "output", 6);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rio_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rio_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rio_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_send(rt, "xxx", 3));
}

