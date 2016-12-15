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
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean, Meng Tan

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFinally
#include "system/redemption_unit_tests.hpp"

#include "utils/sugar/finally.hpp"

BOOST_AUTO_TEST_CASE(TestFinally)
{
    int i = 0;

    try_except(
        [&]{ ++i; },
        [&]{ ++i; }
    );
    BOOST_CHECK_EQUAL(i, 2);

    BOOST_CHECK_THROW(
        try_except(
            [&]{ throw 0; },
            [&]{ ++i; }
        ),
        int
    );

    BOOST_CHECK_THROW(
        rethrow_try_except(
            [&]{ throw 0; },
            [&]{ throw ""; }
        ),
        char const *
    );

    BOOST_CHECK_THROW(
        rethrow_try_except(
            [&]() -> int { throw 0; },
            [&]{ throw ""; }
        ),
        char const *
    );
}
