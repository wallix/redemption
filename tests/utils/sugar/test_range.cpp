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

#define RED_TEST_MODULE TestRange
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/range.hpp"

RED_AUTO_TEST_CASE(TestRange)
{
    char const* p = "test";
    auto end = p + 2;

    auto rng = make_range(p, end);

    RED_CHECK_EQ(rng.size(), 2u);

    auto first = rng.begin();
    auto last = rng.end();

    RED_CHECK(first != last);
    RED_CHECK_EQ(*first, 't');
    ++first;
    RED_CHECK(first != last);
    RED_CHECK_EQ(*first, 'e');
    ++first;
    RED_CHECK(first == last);
}
