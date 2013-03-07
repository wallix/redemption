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
#define BOOST_TEST_MODULE TestOutfileRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "../libs/rio.h"
#include "../libs/rio_impl.h"

// Outfile is a RIO object used to wrap system file writing
// an already opened file descriptor is provided to it
// (Outfile RIO wont open or close this object)

// The same test check both writing and reading as we use RIO infile to read it back
BOOST_AUTO_TEST_CASE(TestOutfileRIO)
{
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_transportXXXXXX");

    {
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);

        RIO_ERROR status = RIO_ERROR_OK;
        RIO * rt = rio_new_outfile(&status, fd);

        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

        char buffer[1024];
        strcpy(buffer, "We read what we provide!");
        BOOST_CHECK_EQUAL(5, rio_send(rt, buffer, 5));
        BOOST_CHECK_EQUAL(19, rio_send(rt, buffer + 5, 19));

        rio_delete(rt);

        ::close(fd);
    }

    {
        int fd = ::open(tmpname, O_RDONLY);

        RIO_ERROR status = RIO_ERROR_OK;
        RIO * rt = rio_new_infile(&status, fd);

        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

        uint8_t buffer[1024];
        
        BOOST_CHECK_EQUAL(3, rio_recv(rt, buffer, 3));
        BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
        BOOST_CHECK_EQUAL(21, rio_recv(rt, buffer + 3, 1024));
        BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
        BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer + 24, 1024));
        
        rio_delete(rt);
    }

    unlink(tmpname);
}
