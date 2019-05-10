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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "transport/out_file_transport.hpp"

#include <cstdlib>


RED_AUTO_TEST_CASE_WF(TestInFileTransport, wf)
{
    {
        OutFileTransport ft(unique_fd{creat(wf.c_str(), O_WRONLY|O_CREAT)});
        ft.send("We write, ", 10);
        ft.send("and again, ", 11);
        ft.send("and so on.", 10);
    }
    RED_CHECK_FCONTENTS(wf.c_str(), "We write, and again, and so on.");
}
