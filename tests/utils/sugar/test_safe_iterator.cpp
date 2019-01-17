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

#define RED_TEST_MODULE TestSafeIterator
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/safe_iterator.hpp"

using Iter = safe_iterator<char const*>;

RED_TEST_DONT_PRINT_LOG_VALUE(Iter)

RED_AUTO_TEST_CASE(TestSafeIterator)
{

    RED_CHECK(!Iter().base());

    char const * const s = "abcd";
    Iter it(s, s+4);
    Iter end = it + 4;
    RED_CHECK_EQ(*it, 'a');
    RED_CHECK_EQ(*end, '\0');
    RED_CHECK_EQ(end - it, 4);
    RED_CHECK_EQ(it[0], 'a');
    RED_CHECK_EQ(it[1], 'b');
    RED_CHECK_EQ(it[2], 'c');
    RED_CHECK_EQ(it[3], 'd');
    ++it;
    RED_CHECK_EQ(*it, 'b');
    RED_CHECK_EQ(end - it, 3);
    RED_CHECK(it < end);
    RED_CHECK(it != end);
    it += 3;
    RED_CHECK_EQ(end - it, 0);
    RED_CHECK(it == end);
    RED_CHECK(!(it < end));
    it -= 2;
    RED_CHECK_EQ(*it, 'c');
    RED_CHECK_EQ(end - it, 2);
    RED_CHECK(it != end);

    it = Iter(s, 4);
    int i = 0;
    for (; it != end; ++it, ++i) {
        RED_CHECK(it != end);
        RED_CHECK(*it);
    }
    RED_CHECK_EQ(i, 4);
}
