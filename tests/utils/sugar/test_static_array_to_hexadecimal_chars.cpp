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

#include "utils/sugar/static_array_to_hexadecimal_chars.hpp"

namespace
{
    template<class... Ts>
    std::array<uint8_t, sizeof...(Ts)> to_array(Ts... xs)
    {
        return std::array<uint8_t, sizeof...(Ts)>{uint8_t(xs)...};
    }
}

RED_AUTO_TEST_CASE(TestStaticArrayHexadecimalChars)
{
    RED_CHECK(static_array_to_hexadecimal_upper_chars(to_array(0x01, 0x23)) == "0123"_av);
    RED_CHECK(static_array_to_hexadecimal_upper_chars(to_array(0x10, 0x23)) == "1023"_av);
    RED_CHECK(static_array_to_hexadecimal_upper_chars(to_array(0x10, 0x23, 0xff)) == "1023FF"_av);
    RED_CHECK(static_array_to_hexadecimal_upper_zchars(to_array(0x10, 0x23, 0xff)) == "1023FF"_av);
    RED_CHECK(static_array_to_hexadecimal_lower_chars(to_array(0x10, 0x23, 0xff)) == "1023ff"_av);
    RED_CHECK(static_array_to_hexadecimal_lower_zchars(to_array(0x10, 0x23, 0xff)) == "1023ff"_av);

    uint8_t a[]{0x10, 0x23, 0xff};
    RED_CHECK(static_array_to_hexadecimal_upper_chars(a) == "1023FF"_av);
    RED_CHECK(static_array_to_hexadecimal_upper_zchars(a) == "1023FF"_av);
    RED_CHECK(static_array_to_hexadecimal_lower_chars(a) == "1023ff"_av);
    RED_CHECK(static_array_to_hexadecimal_lower_zchars(a) == "1023ff"_av);

    RED_CHECK(static_array_to_hexadecimal_upper_chars(to_array(0x10, 0x23, 0xff)).size() == 6);

    auto r = static_array_to_hexadecimal_upper_zchars(to_array(0x10, 0x23, 0xff));
    RED_CHECK(r.size() == 6);
    RED_CHECK(r.c_str()[r.size()] == '\0');
}
