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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>

#include "transport.hpp"
#include "error.hpp"

BOOST_AUTO_TEST_CASE(TestGeneratorTransport)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!", 24);
    char buffer[128] = {};
    char * p = buffer;
    gt.recv(&p, 0);
    BOOST_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    gt.recv(&p, 1);
    BOOST_CHECK_EQUAL(p-buffer, 1);
    BOOST_CHECK_EQUAL(buffer[0], 'W');
    BOOST_CHECK_EQUAL(buffer[1], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 2);
    BOOST_CHECK_EQUAL(p-buffer, 2);
    BOOST_CHECK_EQUAL(buffer[0], 'e');
    BOOST_CHECK_EQUAL(buffer[1], ' ');
    BOOST_CHECK_EQUAL(buffer[2], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 9);
    BOOST_CHECK_EQUAL(p-buffer, 9);
    BOOST_CHECK_EQUAL(0, strncmp(buffer, "read what", 9));

    p = buffer;
    gt.recv(&p, 12);
    BOOST_CHECK_EQUAL(p-buffer, 12);
    BOOST_CHECK_EQUAL(0, strncmp(buffer, " we provide!", 12));

    p = buffer;
    gt.recv(&p, 0);
    BOOST_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    try {
        gt.recv(&p, 1);
        BOOST_CHECK_EQUAL(true, false);
    } catch (Error e) {
        BOOST_CHECK_EQUAL(p-buffer, 0);
        BOOST_CHECK_EQUAL(e.id, (int)ERR_TRANSPORT_NO_MORE_DATA);
    };

}

BOOST_AUTO_TEST_CASE(TestGeneratorTransport2)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!", 24);
    char buffer[128] = {};
    char * p = buffer;
    gt.recv(&p, 0);
    BOOST_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    gt.recv(&p, 1);
    BOOST_CHECK_EQUAL(p-buffer, 1);
    BOOST_CHECK_EQUAL(buffer[0], 'W');
    BOOST_CHECK_EQUAL(buffer[1], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 2);
    BOOST_CHECK_EQUAL(p-buffer, 2);
    BOOST_CHECK_EQUAL(buffer[0], 'e');
    BOOST_CHECK_EQUAL(buffer[1], ' ');
    BOOST_CHECK_EQUAL(buffer[2], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 9);
    BOOST_CHECK_EQUAL(p-buffer, 9);
    BOOST_CHECK_EQUAL(0, strncmp(buffer, "read what", 9));

    p = buffer;
    try {
        gt.recv(&p, 13);
        BOOST_CHECK_EQUAL(true, false);
    } catch (Error e) {
        BOOST_CHECK_EQUAL(p-buffer, 12);
        BOOST_CHECK_EQUAL(0, strncmp(buffer, " we provide!", 12));
        BOOST_CHECK_EQUAL(e.id, (int)ERR_TRANSPORT_NO_MORE_DATA);
    };
}

BOOST_AUTO_TEST_CASE(TestFileTransport)
{
    {
        char tmpname[128];
        sprintf(tmpname, "/tmp/test_transportXXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        {
            OutFileTransport ft(fd);
            ft.send("We write, ", 10);
            ft.send("and again, ", 11);
            ft.send("and so on.", 10);
        }
        ::close(fd);
        fd = ::open(tmpname, O_RDONLY);
        {
            char buf[128];
            char * pbuf = buf;
            InFileTransport ft(fd);
            ft.recv(&pbuf, 10);
            ft.recv(&pbuf, 11);
            ft.recv(&pbuf, 10);
            BOOST_CHECK_EQUAL(0, strncmp(buf, "We write, and again, and so on.", 31));
        }
        ::close(fd);
        ::unlink(tmpname);
    }
}

BOOST_AUTO_TEST_CASE(TestCheckTransport)
{
    CheckTransport gt("input", 5);
    BOOST_CHECK_EQUAL(gt.status, true);
    try{
        gt.send("in", 2);
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK_EQUAL(gt.status, true);
    try{
        gt.send("pot", 3);
    } catch (const Error & e){
        BOOST_CHECK_EQUAL((uint16_t)ERR_TRANSPORT_DIFFERS, (uint16_t)e.id);
    };
    BOOST_CHECK_EQUAL(gt.status, false);
}


BOOST_AUTO_TEST_CASE(TestCheckTransportInputOverflow)
{
    CheckTransport gt("0123456789ABCDEF", 16);
    BOOST_CHECK_EQUAL(gt.status, true);
    try {
        gt.send("0123456789ABCDEFGHI", 19);
    } catch (const Error & e)
    {
        BOOST_CHECK_EQUAL((uint32_t)e.id, (uint32_t)ERR_TRANSPORT_NO_MORE_DATA);
    };
    BOOST_CHECK_EQUAL(gt.status, false);
}

BOOST_AUTO_TEST_CASE(TestTestTransport)
{
    // TestTransport is bidirectional
    // We provide both an output and an input source
    // when using a test Transport we read what we provide in input source
    // and we check that what we write to output is identical to output source
    // if send fails, the difference between expected and actual data is showed
    // and status is set to false (and will stay so) to allow tests to fail.
    // inside Transport, the difference is shown in trace logs.
    TestTransport gt("Test1", "OUTPUT", 6, "input", 5);
    BOOST_CHECK_EQUAL(gt.status, true);
    char buf[128] = {};
    char * p = buf;
    uint32_t sz = 3;
    gt.recv(&p, sz);
    BOOST_CHECK(0 == memcmp(p - sz, "OUT", sz));
    gt.send("in", 2);
    BOOST_CHECK_EQUAL(gt.status, true);
    sz = 3;
    gt.recv(&p, sz);
    BOOST_CHECK(0 == memcmp(p - sz, "PUT", sz));
    try {
        gt.send("pot", 3);
    } catch (const Error & e){
        BOOST_CHECK_EQUAL((uint16_t)ERR_TRANSPORT_DIFFERS, (uint16_t)e.id);
    };
    BOOST_CHECK_EQUAL(gt.status, false);
}

