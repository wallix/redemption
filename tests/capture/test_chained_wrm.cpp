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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"


BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransport)
{
    // setup beforetests
    FileSequence parts("path file pid count extension", "./", "testmeta", "wrm");
    {
        OutByFilenameSequenceTransport setup_wrm(parts);
        for (size_t i = 0 ; i < 10 ; i++){
            char buffer[128];
            sprintf(buffer, "%u", (unsigned)(i*3));
            setup_wrm.send(buffer, strlen(buffer));
            setup_wrm.next();
        }
    }

    // This is whar we are actually testing, chaining of several files content
    InByFilenameSequenceTransport wrm_trans1(parts);
    char buffer[1024];
    char * pbuffer = buffer;
    try {
        wrm_trans1.recv(&pbuffer, 1000);
    } catch (const Error & e) {
        TODO("Is it the right exception ? This one occurs because at some point we do not have another file to provide in the sequence from which to get more data");
        BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_READ_FAILED, (unsigned)e.id);
    };
    *pbuffer = 0;
    BOOST_CHECK(0 == strcmp(buffer, "0369121518212427"));

    // cleanup after tests
    for (size_t i = 0 ; i < 10 ; i++){
        parts.unlink(i);
    }
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportExactPartRead)
{
    // setup beforetests
    FileSequence parts("path file pid count extension", "./", "testmeta", "wrm");
    {
        OutByFilenameSequenceTransport setup_wrm(parts);
        for (size_t i = 0 ; i < 10 ; i++){
            char buffer[128];
            sprintf(buffer, "%u", (unsigned)(i*3));
            setup_wrm.send(buffer, strlen(buffer));
            setup_wrm.next();
        }
    }

    // This is what we are actually testing, chaining of several files content
    InByFilenameSequenceTransport wrm_trans1(parts);
    char buffer[1024] = {};
    char * pbuffer = buffer;

    bool got_exception = false;
    try {
        wrm_trans1.recv(&pbuffer, 1);
        BOOST_CHECK(0 == strcmp(buffer, "0"));
        wrm_trans1.recv(&pbuffer, 1);
        BOOST_CHECK(0 == strcmp(buffer, "03"));
        wrm_trans1.recv(&pbuffer, 1);
        BOOST_CHECK(0 == strcmp(buffer, "036"));
        wrm_trans1.recv(&pbuffer, 1);
        BOOST_CHECK(0 == strcmp(buffer, "0369"));
        wrm_trans1.recv(&pbuffer, 2);
        BOOST_CHECK(0 == strcmp(buffer, "036912"));
        wrm_trans1.recv(&pbuffer, 2);
        BOOST_CHECK(0 == strcmp(buffer, "03691215"));
        wrm_trans1.recv(&pbuffer, 2);
        BOOST_CHECK(0 == strcmp(buffer, "0369121518"));
        wrm_trans1.recv(&pbuffer, 2);
        BOOST_CHECK(0 == strcmp(buffer, "036912151821"));
        wrm_trans1.recv(&pbuffer, 2);
        BOOST_CHECK(0 == strcmp(buffer, "03691215182124"));
        wrm_trans1.recv(&pbuffer, 2);
        BOOST_CHECK(0 == strcmp(buffer, "0369121518212427"));
        wrm_trans1.recv(&pbuffer, 1);
    } catch (const Error & e) {
        BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_READ_FAILED, (unsigned)e.id);
        got_exception = true;
    };

    BOOST_CHECK(0 == strcmp(buffer, "0369121518212427"));
    BOOST_CHECK(got_exception);

    // cleanup after tests
    for (size_t i = 0 ; i < 10 ; i++){
        parts.unlink(i);
    }
}

