/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSQIntracker
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "rio/rio.h"
#include "rio/rio_impl.h"

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

BOOST_AUTO_TEST_CASE(TestSeqIntracker)
{
    // SQIntracker is a Sequence for a chain of files used for reading, defined by an input tracker
    // The tracker starts with three header lines, then lines containing filenames and chunk time limits

    // sq_next() change filename to the next one in the list by reading tracker
    // the next call to sq_get_trans() will open a new trans for reading using this file
    ::unlink("TESTOFS-000000.wrm");
    ::unlink("TESTOFS-000001.wrm");
    ::unlink("TESTOFS-000002.wrm");
    ::unlink("TESTOFS-000003.wrm");

    int fd1 = ::open("TESTOFS-000000.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    ::close(fd1);
    int fd2 = ::open("TESTOFS-000001.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    BOOST_CHECK_EQUAL(fd1, fd2);
    ::close(fd2);
    int fd3 = ::open("TESTOFS-000002.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    BOOST_CHECK_EQUAL(fd1, fd3);
    ::close(fd3);

    char trackerdata[] = "800 600\n"
                      "0\n"
                      "\n"
                      "TESTOFS-000000.wrm 1352304810 1352304841\n"
                      "TESTOFS-000001.wrm 1352304840 1352304861\n"
                      "TESTOFS-000002.wrm 1352304860 1352304881\n"
                    ;
    RIO tracker;
    RIO_ERROR status = rio_init_generator(&tracker, trackerdata, sizeof(trackerdata) - 1);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    timeval tv_begin = {};
    timeval tv_end = {};
    unsigned int chunk_num;
    char path[1024];

    SQ * sq = sq_new_intracker(&status, &tracker, "");
    BOOST_CHECK(sq != NULL);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(1, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "TESTOFS-000000.wrm"));
    BOOST_CHECK_EQUAL(1352304810, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304841, tv_end.tv_sec);

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
    BOOST_CHECK(0 == strcmp(path, "TESTOFS-000001.wrm"));
    BOOST_CHECK_EQUAL(1352304840, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304861, tv_end.tv_sec);

    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));
    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(3, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "TESTOFS-000002.wrm"));
    BOOST_CHECK_EQUAL(1352304860, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304881, tv_end.tv_sec);

    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);

    BOOST_CHECK_EQUAL(RIO_ERROR_EOF, sq_next(sq));

    sq_delete(sq);
    rio_clear(&tracker);

    // tearDown: cleanup files after test
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000000.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000001.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000002.wrm"));
    BOOST_CHECK_EQUAL(-1, ::unlink("TESTOFS-000003.wrm"));
}

BOOST_AUTO_TEST_CASE(TestSeqIntracker1)
{
    // SQIntracker is a Sequence for a chain of files used for reading, defined by an input tracker
    // The tracker starts with three header lines, then lines containing filenames and chunk time limits

    // sq_next() change filename to the next one in the list by reading tracker
    // the next call to sq_get_trans() will open a new trans for reading using this file
    ::unlink("TESTOFS-000000.wrm");
    ::unlink("TESTOFS-000001.wrm");
    ::unlink("TESTOFS-000002.wrm");
    ::unlink("TESTOFS-000003.wrm");

    int fd1 = ::open("TESTOFS-000000.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    ::close(fd1);
    int fd2 = ::open("TESTOFS-000001.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    BOOST_CHECK_EQUAL(fd1, fd2);
    ::close(fd2);
    int fd3 = ::open("TESTOFS-000002.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    BOOST_CHECK_EQUAL(fd1, fd3);
    ::close(fd3);

    char trackerdata[] = "800 600\n"
                         "0\n"
                         "\n"
                         "./tests/fixtures/TESTOFS-000000.wrm 1352304810 1352304841\n"
                         "./tests/fixtures/TESTOFS-000001.wrm 1352304840 1352304861\n"
                         "./tests/fixtures/TESTOFS-000002.wrm 1352304860 1352304881\n"
                       ;
    RIO tracker;
    RIO_ERROR status = rio_init_generator(&tracker, trackerdata, sizeof(trackerdata) - 1);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    timeval tv_begin = {};
    timeval tv_end = {};
    unsigned int chunk_num;
    char path[1024];

    SQ * sq = sq_new_intracker(&status, &tracker, "");
    BOOST_CHECK(sq != NULL);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(1, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "TESTOFS-000000.wrm"));
    BOOST_CHECK_EQUAL(1352304810, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304841, tv_end.tv_sec);

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
    BOOST_CHECK(0 == strcmp(path, "TESTOFS-000001.wrm"));
    BOOST_CHECK_EQUAL(1352304840, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304861, tv_end.tv_sec);

    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));
    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(3, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "TESTOFS-000002.wrm"));
    BOOST_CHECK_EQUAL(1352304860, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304881, tv_end.tv_sec);

    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);

    BOOST_CHECK_EQUAL(RIO_ERROR_EOF, sq_next(sq));

    sq_delete(sq);
    rio_clear(&tracker);

    // tearDown: cleanup files after test
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000000.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000001.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000002.wrm"));
    BOOST_CHECK_EQUAL(-1, ::unlink("TESTOFS-000003.wrm"));
}

BOOST_AUTO_TEST_CASE(TestSeqIntracker2)
{
    // SQIntracker is a Sequence for a chain of files used for reading, defined by an input tracker
    // The tracker starts with three header lines, then lines containing filenames and chunk time limits

    // sq_next() change filename to the next one in the list by reading tracker
    // the next call to sq_get_trans() will open a new trans for reading using this file
    ::unlink("TESTOFS-000000.wrm");
    ::unlink("TESTOFS-000001.wrm");
    ::unlink("TESTOFS-000002.wrm");
    ::unlink("TESTOFS-000003.wrm");

    int fd1 = ::open("TESTOFS-000000.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    ::close(fd1);
    int fd2 = ::open("TESTOFS-000001.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    BOOST_CHECK_EQUAL(fd1, fd2);
    ::close(fd2);
    int fd3 = ::open("TESTOFS-000002.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    BOOST_CHECK_EQUAL(fd1, fd3);
    ::close(fd3);

    char trackerdata[] = "800 600\n"
                         "0\n"
                         "\n"
                         "./TESTOFS-000000.wrm 1352304810 1352304841\n"
                         "./TESTOFS-000001.wrm 1352304840 1352304861\n"
                         "./TESTOFS-000002.wrm 1352304860 1352304881\n"
                       ;
    RIO tracker;
    RIO_ERROR status = rio_init_generator(&tracker, trackerdata, sizeof(trackerdata) - 1);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    timeval tv_begin = {};
    timeval tv_end = {};
    unsigned int chunk_num;
    char path[1024];

    SQ * sq = sq_new_intracker(&status, &tracker, "./tests/fixtures/");
    BOOST_CHECK(sq != NULL);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(1, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "./TESTOFS-000000.wrm"));
    BOOST_CHECK_EQUAL(1352304810, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304841, tv_end.tv_sec);

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
    BOOST_CHECK(0 == strcmp(path, "./TESTOFS-000001.wrm"));
    BOOST_CHECK_EQUAL(1352304840, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304861, tv_end.tv_sec);

    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));
    status = sq_get_chunk_info(sq, &chunk_num, path, sizeof(path), &tv_begin, &tv_end);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    BOOST_CHECK_EQUAL(3, chunk_num);
    BOOST_CHECK(0 == strcmp(path, "./TESTOFS-000002.wrm"));
    BOOST_CHECK_EQUAL(1352304860, tv_begin.tv_sec);
    BOOST_CHECK_EQUAL(1352304881, tv_end.tv_sec);

    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.outfile.fd);

    BOOST_CHECK_EQUAL(RIO_ERROR_EOF, sq_next(sq));

    sq_delete(sq);
    rio_clear(&tracker);

    // tearDown: cleanup files after test
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000000.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000001.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000002.wrm"));
    BOOST_CHECK_EQUAL(-1, ::unlink("TESTOFS-000003.wrm"));
}
