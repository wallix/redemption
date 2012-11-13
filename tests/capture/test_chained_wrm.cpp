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

#define LOGPRINT
#include <sys/time.h>
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"

BOOST_AUTO_TEST_CASE(TestChainedMWRM)
{
    const char * input_filename = "./tests/fixtures/sample.mwrm";
    char buffer[60];
    char * begin = buffer;
    char * end = buffer;
    int fd = ::open(input_filename, O_RDONLY);
    if (fd == -1){
        LOG(LOG_ERR, "Open failed with error %s", strerror(errno));
        throw Error(ERR_TRANSPORT);
    }
    const char * expected[] = {
        "800 600\n",
        "0\n",
        "\n",
        "./tests/fixtures/sample0.wrm, 1352304810 1352304870\n",
        "./tests/fixtures/sample1.wrm, 1352304870 1352304930\n",
        "./tests/fixtures/sample2.wrm, 1352304930 1352304990\n",
    };
    const size_t expected_len[] = {
        8,
        2,
        1,
        52,
        52,
        52,
    };
    const bool has_eol[] = {
        true,
        true,
        true,
        true,
        true,
        true,
    };


    char * eol = NULL;
    for (size_t i=0; i < 6 ; i++){
        bool res = readline(fd, &begin, &end, &eol, buffer, sizeof(buffer));
        BOOST_CHECK_EQUAL(expected_len[i], eol-begin);
        BOOST_CHECK_EQUAL(has_eol[i], res);
        BOOST_CHECK_EQUAL(0, memcmp(expected[i], begin, eol-begin));
        begin = eol;
    }
    ::close(fd);
}


BOOST_AUTO_TEST_CASE(TestChainedMWRMShortBuffer)
{
    const char * input_filename = "./tests/fixtures/sample.mwrm";
    char buffer[30];
    char * begin = buffer;
    char * end = buffer;
    int fd = ::open(input_filename, O_RDONLY);
    if (fd == -1){
        LOG(LOG_ERR, "Open failed with error %s", strerror(errno));
        throw Error(ERR_TRANSPORT);
    }
    const char * expected[] = {
        "800 600\n",
        "0\n",
        "\n",
        "./tests/fixtures/sample0.wrm, ",
        "1352304810 1352304870\n",
        "./tests/fixtures/sample1.wrm, ",
        "1352304870 1352304930\n",
        "./tests/fixtures/sample2.wrm, ",
        "1352304930 1352304990\n",
    };
    const size_t expected_len[] = {
        8,
        2,
        1,
        30,
        22,
        30,
        22,
        30,
        22
    };
    const bool has_eol[] = {
        true,
        true,
        true,
        false,
        true,
        false,
        true,
        false,
        true,
    };

    char * eol = NULL;
    for (size_t i=0; i < 9 ; i++){
        bool res = readline(fd, &begin, &end, &eol, buffer, sizeof(buffer));
        BOOST_CHECK_EQUAL(expected_len[i], eol-begin);
        BOOST_CHECK_EQUAL(has_eol[i], res);
        BOOST_CHECK_EQUAL(0, memcmp(expected[i], begin, eol-begin));
        begin = eol;
    }
    ::close(fd);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransport)
{
    // setup beforetests
    FileSequence parts("path file pid count extension", "./", "testmeta", "wrm");
    {
        OutByFilenameSequenceTransport setup_wrm(parts);
        for (size_t i = 0 ; i < 10 ; i++){
            char buffer[128];
            sprintf(buffer, "%lu", i*3);
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

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM)
{
    // This is what we are actually testing, chaining of several files content
    InByMetaSequenceTransport wrm_trans("./tests/fixtures/sample.mwrm");
    char buffer[10000];
    char * pbuffer = buffer;
    size_t total = 0;
    try {
        for (size_t i = 0; i < 221 ; i++){
//            printf("i=%u\n", (unsigned)i);
            pbuffer = buffer;
            wrm_trans.recv(&pbuffer, sizeof(buffer));
            total += pbuffer - buffer;
        }
    } catch (const Error & e) {
        TODO("Is it the right exception ? This one occurs because at some point we do not have another file to provide in the sequence from which to get more data");
        BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_READ_FAILED, (unsigned)e.id);
        total += pbuffer - buffer;
    };
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(1471394 + 444578 + 290245, total);
}

