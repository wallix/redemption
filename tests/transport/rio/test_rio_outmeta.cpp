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
#define BOOST_TEST_MODULE TestOutMetaRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "rio/rio.h"
#include "rio/rio_impl.h"

// The OutMeta RIO use one inderection level to find out where data should be sent
// the parts are sent to files controled by an outfilename sequence
// a tracker file is also created and the control file list stored int it

// Outmeta is a transport that manage file opening and chunking by itself
// We provide a base filename and it creates an outfilename sequence based on it
// A trace of this sequence is kept in an independant journal file that will
// be used later to reopen the same sequence as an input transport.
// chunking is performed externally, using the independant seq object created by constructor.
// metadata can be attached to individual chunks through seq object.

// The seq object memory allocation is performed by Outmeta,
// hence returned seq *must not* be explicitely deleted
// deleting transport will take care of it.

BOOST_AUTO_TEST_CASE(TestOutMeta)
{
    // cleanup of possible previous test files
    {
        const char * file[] = {"TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
            ::unlink(file[i]);
        }
    }

    RIO_ERROR status = RIO_ERROR_OK;
    SQ * seq  = NULL;
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1352304810;
    const int groupid = 0;
    RIO * rt = rio_new_outmeta(&status, &seq, "", "TESTOFS", ".mwrm", "800 600", "", "", &tv, groupid);

    BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));
    BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));

    rio_delete(rt);

    {
        RIO_ERROR status = RIO_ERROR_OK;
        SQ * sequence = sq_new_inmeta(&status, "./tests/fixtures/TESTOFS", ".mwrm");

        status = RIO_ERROR_OK;
        RIO * rt = rio_new_insequence(&status, sequence);

        char buffer[1024] = {};
        BOOST_CHECK_EQUAL(10, rio_recv(rt, buffer, 10));
        BOOST_CHECK_EQUAL(0, buffer[10]);
        if (0 != memcmp(buffer, "AAAAXBBBBX", 10)){
            LOG(LOG_ERR, "expected \"AAAAXBBBBX\" got \"%s\"\n", buffer);
        }
        BOOST_CHECK_EQUAL(5, rio_recv(rt, buffer + 10, 1024));
        BOOST_CHECK_EQUAL(0, memcmp(buffer, "AAAAXBBBBXCCCCX", 15));
        BOOST_CHECK_EQUAL(0, buffer[15]);
        BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer + 15, 1024));
        rio_clear(rt);
        rio_delete(rt);
        sq_delete(sequence);
    }

    const char * file[] = {
        "TESTOFS.mwrm",
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

BOOST_AUTO_TEST_CASE(TestOutMetaCleaning)
{
    // cleanup of possible previous test files
    {
        const char * file[] = {"TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
            ::unlink(file[i]);
        }
    }

    RIO_ERROR status = RIO_ERROR_OK;
    SQ * seq  = NULL;
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1352304810;
    const int groupid = 0;
    RIO * rt = rio_new_outmeta(&status, &seq, "", "TESTOFS", ".mwrm", "800 600", "", "", &tv, groupid);

    BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));
    BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));

    rio_full_clear(rt);

    const char * file[] = {
        "TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
        if (::unlink(file[i]) >= 0)
        {
            BOOST_CHECK(false);
            LOG(LOG_ERR, "File \"%s\" is always present!", file[i]);
        }
    }
}
