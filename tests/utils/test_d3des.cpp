/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/d3des.hpp"
#include "utils/sugar/cast.hpp"

#include <cstring>

RED_AUTO_TEST_CASE(TestEncryptDes)
{
    rfbDesKey(byte_ptr_cast("12345678"), EN0);

    uint8_t out[8];
    rfbDes(byte_ptr_cast("abcdefgh"), out);
    RED_CHECK(make_array_view(out) == "\x76\xda\xca\xea\xb9\xa5\x28\xa3"_av);

    std::memcpy(out, "abcdefgh", 8);
    rfbDes(out, out);
    RED_CHECK(make_array_view(out) == "\x76\xda\xca\xea\xb9\xa5\x28\xa3"_av);
}

RED_AUTO_TEST_CASE(TestEncryptDesText)
{
    rfbDesKey(byte_ptr_cast("12345678"), EN0);

    uint8_t out[24];
    uint8_t inblock[25] = "abcdefghijklmnopqrstuvwx";
    rfbDesText(inblock, out, 24, byte_ptr_cast("keykeyke"));
    RED_CHECK(make_array_view(out) == "\x29\x42\xca\xbe\xb0\x12\x4b\x68\x98\xef\x52\x55\x79\xcc\xca\xde\x06\x78\xc6\x7d\xa5\x49\x27\xd9"_av_hex);

    std::memcpy(out, "abcdefghijklmnopqrstuvwx", 24);
    rfbDesText(out, out, 24, byte_ptr_cast("keykeyke"));
    RED_CHECK(make_array_view(out) == "\x29\x42\xca\xbe\xb0\x12\x4b\x68\x98\xef\x52\x55\x79\xcc\xca\xde\x06\x78\xc6\x7d\xa5\x49\x27\xd9"_av_hex);
}
