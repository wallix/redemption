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

#include "utils/sugar/int_to_chars.hpp"


RED_AUTO_TEST_CASE(TestIntToDecimalChars)
{
    RED_CHECK(int_to_decimal_chars(0) == "0"_av);
    RED_CHECK(int_to_decimal_chars(1) == "1"_av);
    RED_CHECK(int_to_decimal_chars(9) == "9"_av);
    RED_CHECK(int_to_decimal_chars(10) == "10"_av);
    RED_CHECK(int_to_decimal_chars(99) == "99"_av);
    RED_CHECK(int_to_decimal_chars(100) == "100"_av);
    RED_CHECK(int_to_decimal_chars(123) == "123"_av);
    RED_CHECK(int_to_decimal_chars(-1) == "-1"_av);
    RED_CHECK(int_to_decimal_chars(-123) == "-123"_av);
    RED_CHECK(int_to_decimal_chars(2147483647) == "2147483647"_av);
    RED_CHECK(int_to_decimal_chars(-2147483647) == "-2147483647"_av);
    RED_CHECK(int_to_decimal_chars(-2147483647 - 1) == "-2147483648"_av);
    RED_CHECK(int_to_decimal_chars(9223372036854775807) == "9223372036854775807"_av);
    RED_CHECK(int_to_decimal_chars(-9223372036854775807) == "-9223372036854775807"_av);
    RED_CHECK(int_to_decimal_chars(-9223372036854775807ll - 1) == "-9223372036854775808"_av);
    RED_CHECK(int_to_decimal_chars(18446744073709551615ull) == "18446744073709551615"_av);

    RED_CHECK(int_to_decimal_chars(18446744073709551615ull).size() == 20);

    RED_CHECK(int_to_decimal_zchars(0) == "0"_av);
    RED_CHECK(int_to_decimal_zchars(1) == "1"_av);
    RED_CHECK(int_to_decimal_zchars(123) == "123"_av);
    RED_CHECK(int_to_decimal_zchars(-1) == "-1"_av);
    RED_CHECK(int_to_decimal_zchars(-123) == "-123"_av);
    RED_CHECK(int_to_decimal_zchars(2147483647) == "2147483647"_av);
    RED_CHECK(int_to_decimal_zchars(-2147483647) == "-2147483647"_av);
    RED_CHECK(int_to_decimal_zchars(-2147483647 - 1) == "-2147483648"_av);
    RED_CHECK(int_to_decimal_zchars(9223372036854775807) == "9223372036854775807"_av);
    RED_CHECK(int_to_decimal_zchars(-9223372036854775807) == "-9223372036854775807"_av);
    RED_CHECK(int_to_decimal_zchars(-9223372036854775807ll - 1) == "-9223372036854775808"_av);
    RED_CHECK(int_to_decimal_zchars(18446744073709551615ull) == "18446744073709551615"_av);

    auto d = int_to_decimal_zchars(18446744073709551615ull);
    RED_CHECK(d.size() == 20);
    RED_CHECK(d.c_str()[d.size()] == '\0');

    RED_CHECK(int_to_decimal_chars(static_cast<short>(123)) == "123"_av);
    RED_CHECK(int_to_decimal_chars(static_cast<unsigned short>(123)) == "123"_av);
    RED_CHECK(int_to_decimal_chars(static_cast<char>(123)) == "123"_av);
    RED_CHECK(int_to_decimal_chars(static_cast<signed char>(123)) == "123"_av);
    RED_CHECK(int_to_decimal_chars(static_cast<unsigned char>(123)) == "123"_av);
    RED_CHECK(int_to_decimal_chars(static_cast<short>(-123)) == "-123"_av);
    RED_CHECK(int_to_decimal_chars(static_cast<char>(-123)) == "-123"_av);
    RED_CHECK(int_to_decimal_chars(static_cast<signed char>(-123)) == "-123"_av);
}

RED_AUTO_TEST_CASE(TestIntToHexadecimalChars)
{
    RED_CHECK(int_to_hexadecimal_chars(0x0u) == "0"_av);
    RED_CHECK(int_to_hexadecimal_chars(0x1u) == "1"_av);
    RED_CHECK(int_to_hexadecimal_chars(0x9u) == "9"_av);
    RED_CHECK(int_to_hexadecimal_chars(0xAu) == "A"_av);
    RED_CHECK(int_to_hexadecimal_chars(0xFu) == "F"_av);
    RED_CHECK(int_to_hexadecimal_chars(0x10u) == "10"_av);
    RED_CHECK(int_to_hexadecimal_chars(0x6Bu) == "6B"_av);
    RED_CHECK(int_to_hexadecimal_chars(0x99u) == "99"_av);
    RED_CHECK(int_to_hexadecimal_chars(0xFFFu) == "FFF"_av);
    RED_CHECK(int_to_hexadecimal_chars(0xA39u) == "A39"_av);
    RED_CHECK(int_to_hexadecimal_chars(0xFFFFFFFFu) == "FFFFFFFF"_av);
    RED_CHECK(int_to_hexadecimal_chars(0xFFFFFFFF'FFFFFFFFu) == "FFFFFFFFFFFFFFFF"_av);

    RED_CHECK(int_to_hexadecimal_chars(0xFFFFFFFF'FFFFFFFFu).size() == 16);

    RED_CHECK(int_to_hexadecimal_zchars(0x0u) == "0"_av);
    RED_CHECK(int_to_hexadecimal_zchars(0x1u) == "1"_av);
    RED_CHECK(int_to_hexadecimal_zchars(0x123u) == "123"_av);
    RED_CHECK(int_to_hexadecimal_zchars(0xFFFFFFFFu) == "FFFFFFFF"_av);
    RED_CHECK(int_to_hexadecimal_zchars(0xFFFFFFFF'FFFFFFFFu) == "FFFFFFFFFFFFFFFF"_av);

    auto d = int_to_hexadecimal_zchars(0xFFFFFFFF'FFFFFFFFu);
    RED_CHECK(d.size() == 16);
    RED_CHECK(d.c_str()[d.size()] == '\0');

    RED_CHECK(int_to_hexadecimal_chars(static_cast<unsigned short>(0x123u)) == "123"_av);
    RED_CHECK(int_to_hexadecimal_chars(static_cast<unsigned char>(0x23u)) == "23"_av);
}
