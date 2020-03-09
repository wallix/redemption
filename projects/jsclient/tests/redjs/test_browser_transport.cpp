/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "redjs/browser_transport.hpp"

RED_AUTO_TEST_CASE(TestBrowserTransport)
{
    redjs::BrowserTransport trans;

    RED_CHECK(trans.get_out_buffer() == ""_av);
    trans.add_in_buffer("abc");
    RED_CHECK(trans.get_out_buffer() == ""_av);

    char buffer[10];
    RED_CHECK(3 == trans.partial_read(make_array_view(buffer)));
    RED_CHECK(array_view(buffer, 3) == "abc"_av);
    RED_CHECK_EXCEPTION_ERROR_ID(
        (void)trans.partial_read(make_array_view(buffer)),
        ERR_TRANSPORT_NO_MORE_DATA);

    trans.add_in_buffer("def");
    trans.add_in_buffer("ghi");
    trans.add_in_buffer("jkl");
    trans.add_in_buffer("mno");
    trans.add_in_buffer("pqr");
    trans.add_in_buffer("stu");
    trans.add_in_buffer("vwx");
    trans.add_in_buffer("yz");
    RED_CHECK(3 == trans.partial_read(array_view(buffer, 3)));
    RED_CHECK(array_view(buffer, 3) == "def"_av);
    RED_CHECK(6 == trans.partial_read(array_view(buffer, 6)));
    RED_CHECK(array_view(buffer, 6) == "ghijkl"_av);
    RED_CHECK(5 == trans.partial_read(array_view(buffer, 5)));
    RED_CHECK(array_view(buffer, 5) == "mnopq"_av);
    RED_CHECK(6 == trans.partial_read(array_view(buffer, 6)));
    RED_CHECK(array_view(buffer, 6) == "rstuvw"_av);
    RED_CHECK(3 == trans.partial_read(array_view(buffer, 6)));
    RED_CHECK(array_view(buffer, 3) == "xyz"_av);

    trans.send("123"_av);
    trans.send("456"_av);
    RED_CHECK(trans.get_out_buffer() == "123456"_av);
    trans.clear_out_buffer();
    RED_CHECK(trans.get_out_buffer() == ""_av);
}
