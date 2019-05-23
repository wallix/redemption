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

#include "utils/sugar/static_vector.hpp"

#include <string>

RED_AUTO_TEST_CASE(TestStaticVector)
{
    static_vector<int, 7> v;
    RED_TEST(v.empty());
    RED_TEST(v.size() == 0u);
    RED_TEST(v.capacity() == 7u);
    RED_TEST(v.emplace_back(3) == 3);
    RED_TEST(v.push_back(6) == 6);
    RED_TEST(v.front() == 3);
    RED_TEST(v.back() == 6);
    RED_TEST(v.size() == 2u);
    v.pop_back();
    RED_TEST(v.size() == 1u);
    RED_TEST(v.push_back(9) == 9);
    RED_TEST(v[0] == 3);
    RED_TEST(v[1] == 9);

    auto first = v.begin();
    auto last = v.end();
    RED_TEST((last - first) == 2);
    RED_TEST((first != last));
    RED_TEST((first < last));
    RED_TEST(*first == 3);
    RED_TEST((++first != last));
    RED_TEST(*first == 9);
    RED_TEST((++first == last));
}
