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
#define BOOST_TEST_MODULE TestOutSequenceRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "../libs/rio.h"
#include "../libs/rio_impl.h"

// The Outsequence RIO use one inderection level to find out where data should be sent

// In the test below, we just wrap a check transport in a one_sequence
// hence the resulting outsequence RIO behave exactly like a check RIO
// (see test_rio_check for details)
BOOST_AUTO_TEST_CASE(TestOutSequenceRIO)
{
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1352304810;

    RIO_ERROR status_trans = RIO_ERROR_OK;
    RIO * out = rio_new_check(&status_trans, "AAAAXBBBBXCCCCX", 15);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status_trans);

    RIO_ERROR status_seq = RIO_ERROR_OK;
    SQ * sequence = sq_new_one(&status_seq, out);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status_seq);

    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_outsequence(&status, sequence);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sequence));
    sq_timestamp(sequence, &tv);
    BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));

    rio_delete(rt);
    sq_delete(sequence);
    rio_delete(out);
}

// same test as above with RIO and SQ on stack
BOOST_AUTO_TEST_CASE(TestOutSequenceRIO2)
{
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1352304810;

    RIO out;
    RIO_ERROR status_check = rio_init_check(&out, "AAAAXBBBBXCCCCX", 15);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status_check);

    SQ sequence;
    RIO_ERROR status_seq = sq_init_one(&sequence, &out);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status_seq);

    RIO rt;
    RIO_ERROR status = rio_init_outsequence(&rt, &sequence);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    BOOST_CHECK_EQUAL( 5, rio_send(&rt, "AAAAX",  5));
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(&sequence));
    sq_timestamp(&sequence, &tv);
    BOOST_CHECK_EQUAL(10, rio_send(&rt, "BBBBXCCCCX", 10));

    rio_clear(&rt);
    sq_clear(&sequence);
    rio_clear(&out);
}


BOOST_AUTO_TEST_CASE(TestOutSequenceRIO3)
{
// Second simplest sequence is "outfilename" sequence
// - sq_get_trans() open an outfile if necessary using the given name pattern 
//      and return it on subsequent calls until it is closed
// - sq_next() close the current outfile and step to the next filename wich will 
//    be used by the next sq_get_trans to create an outfile transport.
// - sq_timestamp() : mark the current chunk with timestamp (not useful for sq_outfilename)

// The test below is very similar to the previous one except for the creation of the sequence
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1352304810;
    RIO_ERROR status_seq = RIO_ERROR_OK;
    SQ * sequence = sq_new_outtracker(&status_seq, NULL, SQF_PATH_FILE_COUNT_EXTENSION, "", "TESTOFS", ".wrm", &tv, "800 600", "0", "");

    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_outsequence(&status, sequence);

    BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
    sq_timestamp(sequence, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sequence));
    BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));

    rio_delete(rt);
    
    sq_delete(sequence);

    // tearDown: cleanup files after test
    const char * file[] = {
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
        if (::unlink(file[i]) < 0){
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}
