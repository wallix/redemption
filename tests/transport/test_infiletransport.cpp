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
#define BOOST_TEST_MODULE TestInfileTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "infiletransport.hpp"
#include "error.hpp"

BOOST_AUTO_TEST_CASE(TestInfileTransport)
{
    int fd = ::open("./tests/fixtures/test_infile.txt", O_RDONLY);

    {
        InFileTransport in(fd);

        uint8_t buffer[1024];

        uint8_t * p = buffer;
        in.recv(&p, 3);
        BOOST_CHECK_EQUAL(3, p-buffer);
        BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
        p = buffer + 3;
        try {
            in.recv(&p, 1024);
            BOOST_CHECK(false);
        }
        catch(Error &)
        {
            BOOST_CHECK_EQUAL(24, p-buffer);
            BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
        }
    }

    ::close(fd);
}
