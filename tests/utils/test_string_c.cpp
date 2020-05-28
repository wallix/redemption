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
   Copyright (C) Wallix 2020
   Author(s): Jonathan Poelen
*/

#include "utils/string_c.hpp"

int main()
{
    using namespace jln;
    using namespace jln::literals;

    string_c<'a', 'b'>() = "ab"_c;
    string_c<'a', 'b'>() = "a"_c + "b"_c;
    string_c<'a', 'b'>() = string_c_concat_t<string_c<'a'>, string_c<'b'>>();

    ull_to_string_c_t<0>() = "0"_c;
    ull_to_string_c_t<1>() = "1"_c;
    ull_to_string_c_t<12>() = "12"_c;
    ull_to_string_c_t<123>() = "123"_c;
    ull_to_string_c_t<1234>() = "1234"_c;
    ull_to_string_c_t<12345>() = "12345"_c;
    ull_to_string_c_t<123456>() = "123456"_c;
    ull_to_string_c_t<1234567>() = "1234567"_c;
    ull_to_string_c_t<12345678>() = "12345678"_c;
    ull_to_string_c_t<123456789>() = "123456789"_c;

    integer_to_string_c_t<int, 0>() = "0"_c;
    integer_to_string_c_t<int, 1>() = "1"_c;
    integer_to_string_c_t<int, 12>() = "12"_c;
    integer_to_string_c_t<int, 123>() = "123"_c;
    integer_to_string_c_t<int, 1234>() = "1234"_c;
    integer_to_string_c_t<int, 12345>() = "12345"_c;
    integer_to_string_c_t<int, 123456>() = "123456"_c;
    integer_to_string_c_t<int, 1234567>() = "1234567"_c;
    integer_to_string_c_t<int, 12345678>() = "12345678"_c;
    integer_to_string_c_t<int, 123456789>() = "123456789"_c;
    integer_to_string_c_t<long long, 9223372036854775807>() = "9223372036854775807"_c;

    integer_to_string_c_t<int, -1>() = "-1"_c;
    integer_to_string_c_t<int, -12>() = "-12"_c;
    integer_to_string_c_t<int, -123>() = "-123"_c;
    integer_to_string_c_t<int, -1234>() = "-1234"_c;
    integer_to_string_c_t<int, -12345>() = "-12345"_c;
    integer_to_string_c_t<int, -123456>() = "-123456"_c;
    integer_to_string_c_t<int, -1234567>() = "-1234567"_c;
    integer_to_string_c_t<int, -12345678>() = "-12345678"_c;
    integer_to_string_c_t<int, -123456789>() = "-123456789"_c;
    integer_to_string_c_t<long long, -9223372036854775807ll - 1>()
      = "-9223372036854775808"_c;
}
