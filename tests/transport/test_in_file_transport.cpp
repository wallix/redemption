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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "transport/in_file_transport.hpp"

#include <cstdlib>


RED_AUTO_TEST_CASE(TestInFileTransport)
{
    char buf[128];
    char * pbuf = buf;
    InFileTransport ft(unique_fd{::open(FIXTURES_PATH "/test_infile.txt", O_RDONLY)});
    ft.recv_boom(pbuf, 10);
    pbuf += 10;
    ft.recv_boom(pbuf, 11);
    pbuf += 11;
    ft.recv_boom(pbuf, 3);
    pbuf += 3;
    RED_CHECK_SMEM(array_view(buf, pbuf), "We read what we provide!"_av);
    RED_CHECK_EXCEPTION_ERROR_ID(ft.recv_boom(buf, 1), ERR_TRANSPORT_NO_MORE_DATA);
}
