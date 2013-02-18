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
#define BOOST_TEST_MODULE TestRedTransportLibrary
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "../libs/redtrans.h"

BOOST_AUTO_TEST_CASE(TestGeneratorTransport)
{
    RT_ERROR status = RT_ERROR_OK;
    RT * rt = rt_new_generator(&status, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rt_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rt_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(-RT_ERROR_EOF, rt_recv(rt, buffer+24, 1024));
    
    rt_close(rt);
    rt_delete(rt);
}


BOOST_AUTO_TEST_CASE(TestCheckTransport)
{
    RT_ERROR status = RT_ERROR_OK;
    RT * rt = rt_new_check(&status, "We read what we provide!", 24);
    // Check Transport it somewhat similar to Generator Transport,
    // but instead of allowing to read what is provided
    // it checks that was is written to it is the provided reference string

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(7, rt_send(rt, "We read", 7));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(8, rt_send(rt, " what we!!!", 11));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(-RT_ERROR_DATA_MISMATCH, rt_send(rt, "xxx", 3));
}

BOOST_AUTO_TEST_CASE(TestCheckTransport2)
{
    RT_ERROR status = RT_ERROR_OK;
//    RT * rt = rt_new_test(&status, "output", 6, "input", 5);
    RT * rt = rt_new_check(&status, "output", 6);

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rt_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rt_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rt_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RT_ERROR_TRAILING_DATA, rt_send(rt, "xxx", 3));
}

// TestTransport is basically (and internally) a Generator comined with a Check Transport
// It is designed fot testing purpose of bidirectional transports
// ie:
// - for testing an input file behavior, we will use a Generator
// - for testing an output file behavior, we wll use a Check
// - for testing a socket performing alternated sending and receiving, we use Test Transport
// (obviously a Test transport without input data will behave like Check,
// and a Test Transport without output data will behave like Generator)

BOOST_AUTO_TEST_CASE(TestTestTransport)
{
    RT_ERROR status = RT_ERROR_OK;
//    RT * rt = rt_new_test(&status, "output", 6, "input", 5);
    RT * rt = rt_new_test(&status, "output", 6, "input", 5);

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rt_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rt_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rt_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RT_ERROR_TRAILING_DATA, rt_send(rt, "xxx", 3));
}


BOOST_AUTO_TEST_CASE(TestFileTransport)
{
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_transportXXXXXX");

    {
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        RT_ERROR status = RT_ERROR_OK;
        RT * rt = rt_new_outfile(&status, fd);
        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RT_ERROR_OK, status);

        char buffer[1024];
        strcpy(buffer, "We read what we provide!");
        BOOST_CHECK_EQUAL(5, rt_send(rt, buffer, 5));
        BOOST_CHECK_EQUAL(19, rt_send(rt, buffer + 5, 19));

        rt_close(rt);
        rt_delete(rt);

        ::close(fd);
    }

    {
        int fd = ::open(tmpname, O_RDONLY);

        RT_ERROR status = RT_ERROR_OK;
        RT * rt = rt_new_infile(&status, fd);
        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RT_ERROR_OK, status);

        uint8_t buffer[1024];
        
        BOOST_CHECK_EQUAL(3, rt_recv(rt, buffer, 3));
        BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
        BOOST_CHECK_EQUAL(21, rt_recv(rt, buffer + 3, 1024));
        BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
        BOOST_CHECK_EQUAL(0, rt_recv(rt, buffer + 24, 1024));
        
        rt_close(rt);
        rt_delete(rt);
    }
}

