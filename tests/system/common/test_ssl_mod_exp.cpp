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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include "system/redemption_unit_tests.hpp"

#include "check_mem.hpp"

BOOST_AUTO_TEST_CASE(TestModExp)
{
    char inr[] = "\xc";
    char modulus[] = "\x3";
    char exponent[] = "\x4";

    uint8_t out[sizeof(modulus)];


    // 12^4 % 3 = 48 % 3 = 0
    size_t len = mod_exp(
        out, sizeof(out)
      , reinterpret_cast<uint8_t const *>(inr), sizeof(inr)-1
      , reinterpret_cast<uint8_t const *>(modulus), sizeof(modulus)-1
      , reinterpret_cast<uint8_t const *>(exponent), sizeof(exponent)-1
    );

    BOOST_CHECK_EQUAL(len, 0);


    // 12^4 % 5 = 48 % 5 = 3
    modulus[0] = '\x5';
    len = mod_exp(
        out, sizeof(out)
      , reinterpret_cast<uint8_t const *>(inr), sizeof(inr)-1
      , reinterpret_cast<uint8_t const *>(modulus), sizeof(modulus)-1
      , reinterpret_cast<uint8_t const *>(exponent), sizeof(exponent)-1
    );

    BOOST_CHECK_EQUAL(len, 1);
    CHECK_MEM(out, 1, "\x1");


    // 12^4 % 17 = 48 % 17 = 13
    char modulus2[] = "\x11";
    len = mod_exp(
        out, sizeof(out)
      , reinterpret_cast<uint8_t const *>(inr), sizeof(inr)-1
      , reinterpret_cast<uint8_t const *>(modulus2), sizeof(modulus2)-1
      , reinterpret_cast<uint8_t const *>(exponent), sizeof(exponent)-1
    );

    BOOST_CHECK_EQUAL(len, 1);
    CHECK_MEM(out, 1, "\xd");
}


BOOST_AUTO_TEST_CASE(TestBigModExp)
{
    char inr[] = "c9bt8v6pbtr73";
    char modulus[] = "6TBD*S^0b5F*^%";
    char exponent[] = "f89sn6B*(FD(bf5sd969g";
    uint8_t out[sizeof(modulus)];

    size_t len = mod_exp(
        out, sizeof(out)
      , reinterpret_cast<uint8_t const *>(inr), sizeof(inr)-1
      , reinterpret_cast<uint8_t const *>(modulus), sizeof(modulus)-1
      , reinterpret_cast<uint8_t const *>(exponent), sizeof(exponent)-1
    );

    BOOST_CHECK_EQUAL(len, sizeof(modulus)-1);
    CHECK_MEM(
        out, sizeof(modulus) - 1,
        "\x1e\xc0\x4d\xea\xbd\xc5\x25\x19\x71\xa6\x69\x1d\x3a\x82"
    );
}
