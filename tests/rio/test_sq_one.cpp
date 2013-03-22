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
#define BOOST_TEST_MODULE TestSQOne
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include"log.hpp"

#include"rio/rio.h"
#include"rio/rio_impl.h"

// Sequence are used to control chaining of chunked RIO
// Insequence provide RIOs that will be used for reading
// sq_get_trans(sq) : returns the current RIO
// sq_next(sq)      : goes to next RIO (the next call to sq_get_trans will change RIO)

// the 2 API functions below are currently tailored to the need of video traces
// providing some access to all purpose meta informations attached to chunks
// could probably be of interest.

// sq_timestamp(sq, tv) : set current timestamp, useful only for (some) output sequence
//    (the last timestamp setted will be used as the end timestamp of the current chunk if needed)
//    (and as the first timestamp of the next open chunk)
// sq_get_chunk_info(sq, &num, path, path_len, &tv_begin, &tv_end) : useful only for (some) input sequence
// returns informations on the current read chunk (the chunk currently returned by get_trans)
// num get the chunk number (1 for the first chunk)
// path returns the name of the chunk (ie: filename)
// tv_begin and tv_end are start and stop of chunk


BOOST_AUTO_TEST_CASE(TestSeqOne)
{
    // SQOne is a Sequence wrapping a single RIO in a sequence
    // It does not change current RIO when sq_next is called
    // it is useful to interface a single target RIO with code written to work with sequence
    // (like RIOInsequence). Data that would otherwise be splitted between several chunks
    // will be sent to the same one.

    RIO rt;
    RIO_ERROR status = rio_init_generator(&rt, "We read what we provide!", 24);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    
    SQ * sq = sq_new_one(&status, &rt);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    BOOST_CHECK_EQUAL(&rt, sq_get_trans(sq, &status));
    BOOST_CHECK_EQUAL(&rt, sq_get_trans(sq, &status));
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));
    BOOST_CHECK_EQUAL(&rt, sq_get_trans(sq, &status));
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));
    BOOST_CHECK_EQUAL(&rt, sq_get_trans(sq, &status));

    sq_delete(sq);
    rio_clear(&rt);
}

BOOST_AUTO_TEST_CASE(TestSeqOne2)
{
    // Same as above using provided SQ on stack

    RIO rt;
    RIO_ERROR status = rio_init_generator(&rt, "We read what we provide!", 24);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    
    SQ sq;
    status = sq_init_one(&sq, &rt);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    BOOST_CHECK_EQUAL(&rt, sq_get_trans(&sq, &status));
    BOOST_CHECK_EQUAL(&rt, sq_get_trans(&sq, &status));
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(&sq));
    BOOST_CHECK_EQUAL(&rt, sq_get_trans(&sq, &status));
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(&sq));
    BOOST_CHECK_EQUAL(&rt, sq_get_trans(&sq, &status));

    sq_clear(&sq);
    rio_clear(&rt);
}

