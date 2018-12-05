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

#define RED_TEST_MODULE TestSplitter
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/splitter.hpp"
#include "utils/sugar/algostring.hpp"
#include <string>

RED_AUTO_TEST_CASE(TestSplitter)
{
    const char text[] = "abc,de,efg,h,ijk,lmn";
    std::string s;
    for (auto r : get_line(text, ',')) {
        s.append(r.begin(), r.end()) += ':';
    }
    RED_CHECK_EQUAL(s, "abc:de:efg:h:ijk:lmn:");

    s.clear();
    std::string stest(text);
    for (auto r : get_split(stest, ',')) {
        s.append(r.begin(), r.end()) += ':';
    }
    RED_CHECK_EQUAL(s, "abc:de:efg:h:ijk:lmn:");
}

RED_AUTO_TEST_CASE(TestSplitter2)
{
    const char * drives = " export ,, , \t share \t ,";

    std::string s;
    for (auto r : get_line(drives, ',')) {
        auto trimmed_range = trim(r);

        if (trimmed_range.empty()) continue;

        s.append(begin(trimmed_range), end(trimmed_range)) += ',';
    }
    RED_CHECK_EQUAL(s, "export,share,");
}
