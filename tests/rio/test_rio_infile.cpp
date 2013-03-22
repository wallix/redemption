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
#define BOOST_TEST_MODULE TestXXXXXRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "rio/rio.h"
#include "rio/rio_impl.h"
#include "../utils/fileutils.hpp"


// Infile is a RIO object used to wrap system file reading
// an already opened file descriptor is provided to it
// (Infile RIO wont open or close this fd)

// RIO infile read data from fixture object
BOOST_AUTO_TEST_CASE(TestOutfileRIO)
{
    int fd = ::open("./tests/fixtures/test_infile.txt", O_RDONLY);

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
    BOOST_CHECK_EQUAL(0, close(fd));
}


