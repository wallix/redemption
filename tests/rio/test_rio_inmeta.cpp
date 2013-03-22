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
#define BOOST_TEST_MODULE TestInmetaRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include"log.hpp"

#include"rio/rio.h"
#include"rio/rio_impl.h"

BOOST_AUTO_TEST_CASE(TestInmeta)
{
    // cleanup of possible previous test files
    {
        const char * file[] = {"TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
            ::unlink(file[i]);
        }
    }

    {
        RIO_ERROR status = RIO_ERROR_OK;
        SQ * seq  = NULL;
        struct timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        RIO * rt = rio_new_outmeta(&status, &seq, "", "TESTOFS", ".mwrm", "800 600", "", "", &tv);

        BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
        tv.tv_sec+= 100;
        sq_timestamp(seq, &tv);
        BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));
        BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));
        tv.tv_sec+= 100;
        sq_timestamp(seq, &tv);

        rio_clear(rt);
        rio_delete(rt);
    }

    {
        SQ * inseq = NULL;
        RIO_ERROR status = RIO_ERROR_OK;
        RIO * rt = rio_new_inmeta(&status, &inseq, "TESTOFS", ".mwrm");
        BOOST_CHECK( rt != NULL);

        char buffer[1024] = {};
        BOOST_CHECK_EQUAL(15, rio_recv(rt, buffer,  15));
        if (0 != memcmp(buffer, "AAAAXBBBBXCCCCX", 15)){
            BOOST_CHECK_EQUAL(0, buffer[15]); // this one should not have changed
            buffer[15] = 0;
            LOG(LOG_ERR, "expected \"AAAAXBBBBXCCCCX\" got \"%s\"", buffer);
            BOOST_CHECK(false);
        }
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

BOOST_AUTO_TEST_CASE(TestInmeta2)
{
        RIO_ERROR status = RIO_ERROR_OK;
        SQ * seq = NULL;
        RIO * rio = rio_new_inmeta(&status, &seq, "TESTOFSXXX", ".mwrm");
        rio_delete(rio);
}
