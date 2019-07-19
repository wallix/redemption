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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "utils/sugar/cast.hpp"

#include <array>


RED_AUTO_TEST_CASE(TestModExp)
{
    constexpr auto inr = "\xc"_av;
    constexpr auto modulus = "\x3"_av;
    constexpr auto exponent = "\x4"_av;

    std::array<uint8_t, modulus.size()+1> out;

    // 12^4 % 3 = 20736 % 3 = 0
    RED_CHECK_MEM(mod_exp(out, inr, modulus, exponent), ""_av);

    // 12^4 % 5 = 20736 % 5 = 1
    RED_CHECK_MEM(mod_exp(out, inr, "\x5"_av, exponent), "\x1"_av);

    // 12^4 % 17 = 20736 % 17 = 13
    RED_CHECK_MEM(mod_exp(out, inr, "\x11"_av, exponent), "\xd"_av);
}


RED_AUTO_TEST_CASE(TestBigModExp)
{
    constexpr auto inr = "c9bt8v6pbtr73"_av;
    constexpr auto modulus = "6TBD*S^0b5F*^%"_av;
    constexpr auto exponent = "f89sn6B*(FD(bf5sd969g"_av;

    std::array<uint8_t, modulus.size()+1> out;

    RED_CHECK_MEM(mod_exp(out, inr, modulus, exponent),
        "\x1e\xc0\x4d\xea\xbd\xc5\x25\x19\x71\xa6\x69\x1d\x3a\x82"_av);
}
