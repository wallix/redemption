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

#define RED_TEST_MODULE TestFinally
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/finally.hpp"

RED_AUTO_TEST_CASE(TestFinally)
{
    int i = 0;

    try_except(
        [&]{ ++i; },
        [&]{ ++i; }
    );
    RED_CHECK_EQUAL(i, 2);

    RED_CHECK_THROW(
        try_except(
            [&]{ throw 0; },
            [&]{ ++i; }
        ),
        int
    );

    RED_CHECK_THROW(
        rethrow_try_except(
            [&]{ throw 0; },
            [&]{ throw ""; }
        ),
        char const *
    );

    RED_CHECK_THROW(
        rethrow_try_except(
            [&]() -> int { throw 0; },
            [&]{ throw ""; }
        ),
        char const *
    );
}
