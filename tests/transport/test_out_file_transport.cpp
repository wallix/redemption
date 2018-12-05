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

#define RED_TEST_MODULE TestInFileTransport
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "transport/out_file_transport.hpp"
#include "test_only/get_file_contents.hpp"

#include <cstdlib>


RED_AUTO_TEST_CASE(TestInFileTransport)
{
    char tmpname[128] = "/tmp/test_transportXXXXXX";
    {
        OutFileTransport ft(unique_fd{::mkostemp(tmpname, O_WRONLY|O_CREAT)});
        ft.send("We write, ", 10);
        ft.send("and again, ", 11);
        ft.send("and so on.", 10);
    }
    RED_CHECK_EQUAL(get_file_contents(tmpname), "We write, and again, and so on.");
    ::unlink(tmpname);
}
