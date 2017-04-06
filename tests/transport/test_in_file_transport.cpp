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
   Copyright (C) Wallix 2013-2016
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOutFileTransport
#include "system/redemption_unit_tests.hpp"

#define LOGPRINT
#include "utils/log.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "transport/in_file_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "core/error.hpp"

BOOST_AUTO_TEST_CASE(TestOutFileTransport)
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
            ft.recv_atomic(pbuf, 10);
            pbuf += 10;
            ft.recv_atomic(pbuf, 11);
            pbuf += 11;
            ft.recv_atomic(pbuf, 10);
            pbuf += 10;
            BOOST_CHECK_EQUAL(0, strncmp(buf, "We write, and again, and so on.", 31));
            pbuf = buf;
            CHECK_EXCEPTION_ERROR_ID(ft.recv_atomic(pbuf, 1), ERR_TRANSPORT_NO_MORE_DATA);
        }
        ::close(fd);
        ::unlink(tmpname);
    }
}

