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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_SSL_BIGNUM
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
#define LOGPRINT

#include "system/ssl_bignum.hpp"

BOOST_AUTO_TEST_CASE(TestAdditionSimple)
{
    Bignum bn(100);
    BOOST_CHECK_EQUAL(bn.get_word(), Bignum(100).get_word());
    BOOST_CHECK(bn == Bignum(100));
    Bignum bn2(234);
    Bignum res = bn + bn2;
    Bignum expected(334);
    //BOOST_CHECK_EQUAL(static_cast<int>(BN_cmp(res.n, expected.n)), static_cast<int>(0));
    BOOST_CHECK(res == expected);
}

// (12345 ** 15) = 23570370761335746188692151519494473063659327922288482666015625L
// (12345 ** 15) % 100 = 25

BOOST_AUTO_TEST_CASE(TestModExpSimple)
{
    Bignum a(12345);
    Bignum exp(15);
    Bignum mod(100);
    Bignum res = a.mod_exp(exp, mod);
    Bignum expected(25);
    //BOOST_CHECK_EQUAL(res.get_word(), 0);
    BOOST_CHECK(res == expected);
}
