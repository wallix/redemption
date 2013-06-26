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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestInByMetaSequenceTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>

#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"
#include "inbymetasequencetransport.hpp"
#include "error.hpp"




BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1)
{
    // This is what we are actually testing, chaining of several files content
    InByMetaSequenceTransport wrm_trans("./tests/fixtures/sample", ".mwrm");
    char buffer[10000];
    char * pbuffer = buffer;
    size_t total = 0;
    try {
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv(&pbuffer, sizeof(buffer));
            total += pbuffer - buffer;
        }
    } catch (const Error & e) {
        BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_NO_MORE_DATA, (unsigned)e.id);
        total += pbuffer - buffer;
    };
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(1471394 + 444578 + 290245, total);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "./tests/fixtures/sample0.wrm 1352304810 1352304870\n",
//        "./tests/fixtures/sample1.wrm 1352304870 1352304930\n",
//        "./tests/fixtures/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    {
        InByMetaSequenceTransport mwrm_trans("./tests/fixtures/sample", ".mwrm");
        BOOST_CHECK_EQUAL(0, mwrm_trans.chunk_num);

        mwrm_trans.next_chunk_info();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path);
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time);
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time);
        BOOST_CHECK_EQUAL(1, mwrm_trans.chunk_num);

        mwrm_trans.next_chunk_info();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path);
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time);
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time);
        BOOST_CHECK_EQUAL(2, mwrm_trans.chunk_num);

        mwrm_trans.next_chunk_info();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path);
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time);
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time);
        BOOST_CHECK_EQUAL(3, mwrm_trans.chunk_num);

        try {
            mwrm_trans.next_chunk_info();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_READ_FAILED, e.id);
            BOOST_CHECK(true);
        };
    }

    // check we can do it two times    
    InByMetaSequenceTransport mwrm_trans("./tests/fixtures/sample", ".mwrm");

    BOOST_CHECK_EQUAL(0, mwrm_trans.chunk_num);

    mwrm_trans.next_chunk_info();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path);
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time);
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time);
    BOOST_CHECK_EQUAL(1, mwrm_trans.chunk_num);

    mwrm_trans.next_chunk_info();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path);
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time);
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time);
    BOOST_CHECK_EQUAL(2, mwrm_trans.chunk_num);

    mwrm_trans.next_chunk_info();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path);
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time);
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time);
    BOOST_CHECK_EQUAL(3, mwrm_trans.chunk_num);
    
}


BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2_RIO)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "./tests/fixtures/sample0.wrm 1352304810 1352304870\n",
//        "./tests/fixtures/sample1.wrm 1352304870 1352304930\n",
//        "./tests/fixtures/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    try {
        InByMetaSequenceTransport mwrm_trans("./tests/fixtures/sample", ".mwrm");
        BOOST_CHECK_EQUAL(0, mwrm_trans.chunk_num);

        mwrm_trans.next_chunk_info();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path);
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time);
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time);
        BOOST_CHECK_EQUAL(1, mwrm_trans.chunk_num);

        mwrm_trans.next_chunk_info();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path);
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time);
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time);
        BOOST_CHECK_EQUAL(2, mwrm_trans.chunk_num);

        mwrm_trans.next_chunk_info();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path);
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time);
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time);
        BOOST_CHECK_EQUAL(3, mwrm_trans.chunk_num);

        try {
            mwrm_trans.next_chunk_info();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_READ_FAILED, e.id);
            BOOST_CHECK(true);
        };

    } catch(const Error & e) {
        BOOST_CHECK(false);
    };

}

