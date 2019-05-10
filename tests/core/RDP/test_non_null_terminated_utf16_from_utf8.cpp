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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "core/RDP/non_null_terminated_utf16_from_utf8.hpp"


RED_AUTO_TEST_CASE(Test_non_null_terminated_utf16_from_utf8)
{
    char buf[]{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'};
    OutStream out_stream(buf);
    RED_TEST(4 == put_non_null_terminated_utf16_from_utf8(out_stream, "AB"_av, out_stream.get_capacity()));
    RED_CHECK_SMEM(array_view_const_char(buf, 6), "\x04\00""A\0B\0"_av);
}
