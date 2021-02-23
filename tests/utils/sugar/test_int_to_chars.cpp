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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   Unit test for color conversion primitives

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/int_to_chars.hpp"


RED_AUTO_TEST_CASE(TestIntToChars)
{
    RED_CHECK(int_to_chars(0) == "0"_av);
    RED_CHECK(int_to_chars(1) == "1"_av);
    RED_CHECK(int_to_chars(123) == "123"_av);
    RED_CHECK(int_to_chars(-1) == "-1"_av);
    RED_CHECK(int_to_chars(-123) == "-123"_av);
    RED_CHECK(int_to_chars(2147483647) == "2147483647"_av);
    RED_CHECK(int_to_chars(-2147483647) == "-2147483647"_av);
    RED_CHECK(int_to_chars(-2147483647 - 1) == "-2147483648"_av);
    RED_CHECK(int_to_chars(9223372036854775807) == "9223372036854775807"_av);
    RED_CHECK(int_to_chars(-9223372036854775807) == "-9223372036854775807"_av);
    RED_CHECK(int_to_chars(-9223372036854775807ll - 1) == "-9223372036854775808"_av);
    RED_CHECK(int_to_chars(18446744073709551615ull) == "18446744073709551615"_av);

    RED_CHECK(int_to_chars(18446744073709551615ull).size() == 20);

    RED_CHECK(int_to_zchars(0) == "0"_av);
    RED_CHECK(int_to_zchars(1) == "1"_av);
    RED_CHECK(int_to_zchars(123) == "123"_av);
    RED_CHECK(int_to_zchars(-1) == "-1"_av);
    RED_CHECK(int_to_zchars(-123) == "-123"_av);
    RED_CHECK(int_to_zchars(2147483647) == "2147483647"_av);
    RED_CHECK(int_to_zchars(-2147483647) == "-2147483647"_av);
    RED_CHECK(int_to_zchars(-2147483647 - 1) == "-2147483648"_av);
    RED_CHECK(int_to_zchars(9223372036854775807) == "9223372036854775807"_av);
    RED_CHECK(int_to_zchars(-9223372036854775807) == "-9223372036854775807"_av);
    RED_CHECK(int_to_zchars(-9223372036854775807ll - 1) == "-9223372036854775808"_av);
    RED_CHECK(int_to_zchars(18446744073709551615ull) == "18446744073709551615"_av);

    auto d = int_to_zchars(18446744073709551615ull);
    RED_CHECK(d.size() == 20);
    RED_CHECK(d.c_str()[d.size()] == '\0');
}
