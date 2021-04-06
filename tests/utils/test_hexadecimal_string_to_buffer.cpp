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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/hexadecimal_string_to_buffer.hpp"

RED_AUTO_TEST_CASE(TestHexToBuf)
{
    auto input = "AB18F012"_av;
    uint8_t output[4];
    RED_CHECK(hexadecimal_string_to_buffer(input, make_writable_array_view(output)));
    RED_CHECK(make_writable_array_view(output) == "\xAB\x18\xF0\x12"_av);

    RED_CHECK(!hexadecimal_string_to_buffer("AB18F0_1"_av, output));
}
