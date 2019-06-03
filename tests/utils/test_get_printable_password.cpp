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
#include "utils/get_printable_password.hpp"


RED_AUTO_TEST_CASE(TestPrintable)
{
    auto pass = "pass"_av;
    auto empty = ""_av;
    auto hidden = "<hidden>"_av;
    auto null = "<null>"_av;

    RED_CHECK_SMEM(hidden, get_printable_password(pass, 0));
    RED_CHECK_SMEM(hidden, get_printable_password(pass, 1));
    RED_CHECK_SMEM(pass, get_printable_password(pass, 2));
    RED_CHECK_SMEM(hidden, get_printable_password(pass, 3));

    RED_CHECK_SMEM(hidden, get_printable_password(empty, 0));
    RED_CHECK_SMEM(null, get_printable_password(empty, 1));
    RED_CHECK_SMEM(empty, get_printable_password(empty, 2));
    RED_CHECK_SMEM(hidden, get_printable_password(empty, 3));
}
