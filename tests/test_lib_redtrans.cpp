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
#define BOOST_TEST_MODULE TestRedTransportLibrary
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "../libs/redtrans.h"

BOOST_AUTO_TEST_CASE(TestGeneratorTransport)
{
    RT * rt = rt_new(RT_TYPE_GENERATOR);
    BOOST_CHECK(NULL != rt);

    unsigned status = rt_init_generator(rt, "We read what we provide!", 24);
    BOOST_CHECK_EQUAL(0, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rt_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rt_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(0, rt_recv(rt, buffer+24, 1024));
    
    rt_close(rt);
    rt_delete(rt);
}

BOOST_AUTO_TEST_CASE(TestFileTransport)
{
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_transportXXXXXX");

    {
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        RT * rt = rt_new(RT_TYPE_OUTFILE);
        BOOST_CHECK(NULL != rt);

        unsigned status = rt_init_outfile_writer(rt, fd);
        BOOST_CHECK_EQUAL(0, status);

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

        RT * rt = rt_new(RT_TYPE_INFILE);
        BOOST_CHECK(NULL != rt);

        unsigned status = rt_init_infile_reader(rt, fd);
        BOOST_CHECK_EQUAL(0, status);

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

