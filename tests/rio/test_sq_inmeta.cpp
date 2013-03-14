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
#define BOOST_TEST_MODULE TestSQInmeta
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "../libs/rio.h"
#include "../libs/rio_impl.h"

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


BOOST_AUTO_TEST_CASE(TestSeqInmeta)
{
    // SQIntracker is a Sequence for a chain of files used for reading, defined by an input tracker
    // The tracker starts with three header lines, then lines containing filenames and chunk time limits

    // sq_next() change filename to the next one in the list by reading tracker
    // the next call to sq_get_trans() will open a new trans for reading using this file

    BOOST_CHECK(true); // do not remove : this test is to force opening boost test pipe before performing open
    timeval tv_begin;
    timeval tv_end;
    unsigned int chunk_num;
    char path[1024];    

    RIO_ERROR status;
    SQ * sq = sq_new_inmeta(&status, "./tests/fixtures/sample", ".mwrm");
    BOOST_CHECK(sq != NULL);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    int fd1 = ::open("tmp.tmp", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    ::close(fd1);
    ::unlink("tmp.tmp");

    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(1, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "./tests/fixtures/sample0.wrm"));
    BOOST_CHECK_EQUAL(1352304810, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304870, tv_end.tv_sec);  

    RIO * rt = NULL;
    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);
    
    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);
    
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));
    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(2, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "./tests/fixtures/sample1.wrm"));
    BOOST_CHECK_EQUAL(1352304870, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304930, tv_end.tv_sec);  
    
    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);
    
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));
    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(3, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "./tests/fixtures/sample2.wrm"));
    BOOST_CHECK_EQUAL(1352304930, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304990, tv_end.tv_sec);  

    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);

    BOOST_CHECK_EQUAL(RIO_ERROR_EOF, sq_next(sq));

    sq_delete(sq);
}

