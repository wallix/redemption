/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/


#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/base64.hpp"

RED_AUTO_TEST_CASE(TestEncode64)
{
    char buffer[250];
    auto base64_encode = [&](array_view_const_char av){
        return ::base64_encode(av, make_array_view(buffer));
    };
    RED_CHECK(base64_encode("any carnal pleasure."_av) == "YW55IGNhcm5hbCBwbGVhc3VyZS4="_av);
    RED_CHECK(base64_encode("any carnal pleasure"_av) ==  "YW55IGNhcm5hbCBwbGVhc3VyZQ=="_av);
    RED_CHECK(base64_encode("any carnal pleasur"_av) ==   "YW55IGNhcm5hbCBwbGVhc3Vy"_av);
    RED_CHECK(base64_encode("any carnal pleasu"_av) ==    "YW55IGNhcm5hbCBwbGVhc3U="_av);
    RED_CHECK(base64_encode("any carnal pleas"_av) ==     "YW55IGNhcm5hbCBwbGVhcw=="_av);
    RED_CHECK(base64_encode("pleasure."_av) == "cGxlYXN1cmUu"_av);
    RED_CHECK(base64_encode("leasure."_av) ==  "bGVhc3VyZS4="_av);
    RED_CHECK(base64_encode("easure."_av) ==   "ZWFzdXJlLg=="_av);
    RED_CHECK(base64_encode("asure."_av) ==    "YXN1cmUu"_av);
    RED_CHECK(base64_encode("sure."_av) ==     "c3VyZS4="_av);
    RED_CHECK(base64_encode("sure"_av) == "c3VyZQ=="_av);
    RED_CHECK(base64_encode("sur"_av) ==  "c3Vy"_av);
    RED_CHECK(base64_encode("su"_av) ==   "c3U="_av);
    RED_CHECK(base64_encode("s"_av) ==    "cw=="_av);
    RED_CHECK(base64_encode(""_av) ==     ""_av);
    RED_CHECK(base64_encode(
        "\xb3\x7a\x4f\x2c\xc0\x62\x4f\x16\x90\xf6\x46\x06\xcf\x38\x59\x45\xb2\xbe\xc4\xea"_av) ==
        "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="_av);
}
