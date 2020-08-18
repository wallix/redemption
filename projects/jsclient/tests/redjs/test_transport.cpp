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

#include "redjs/transport.hpp"

RED_AUTO_TEST_CASE(TestTransport)
{
    redjs::Transport trans;

    RED_CHECK(trans.get_output_buffer() == ""_av);
    trans.push_input_buffer("abc");
    RED_CHECK(trans.get_output_buffer() == ""_av);

    char buffer[10];
    RED_CHECK(3 == trans.partial_read(make_writable_array_view(buffer)));
    RED_CHECK(array_view(buffer, 3) == "abc"_av);
    RED_CHECK_EXCEPTION_ERROR_ID(
        (void)trans.partial_read(make_writable_array_view(buffer)),
        ERR_TRANSPORT_NO_MORE_DATA);

    trans.push_input_buffer("def");
    trans.push_input_buffer("ghi");
    trans.push_input_buffer("jkl");
    trans.push_input_buffer("mno");
    trans.push_input_buffer("pqr");
    trans.push_input_buffer("stu");
    trans.push_input_buffer("vwx");
    trans.push_input_buffer("yz");
    RED_CHECK(3 == trans.partial_read(writable_array_view(buffer, 3)));
    RED_CHECK(array_view(buffer, 3) == "def"_av);
    RED_CHECK(6 == trans.partial_read(writable_array_view(buffer, 6)));
    RED_CHECK(array_view(buffer, 6) == "ghijkl"_av);
    RED_CHECK(5 == trans.partial_read(writable_array_view(buffer, 5)));
    RED_CHECK(array_view(buffer, 5) == "mnopq"_av);
    RED_CHECK(6 == trans.partial_read(writable_array_view(buffer, 6)));
    RED_CHECK(array_view(buffer, 6) == "rstuvw"_av);
    RED_CHECK(3 == trans.partial_read(writable_array_view(buffer, 6)));
    RED_CHECK(array_view(buffer, 3) == "xyz"_av);

    trans.send("123"_av);
    trans.send("456"_av);
    RED_CHECK(trans.get_output_buffer() == "123456"_av);
    trans.clear_output_buffer();
    RED_CHECK(trans.get_output_buffer() == ""_av);
}
