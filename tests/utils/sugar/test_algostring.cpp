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

#include "utils/sugar/algostring.hpp"
#include <string>

RED_AUTO_TEST_CASE(TestTrim)
{
    char const s[] = " \t abcd   ";
    auto left_s = s+3;
    auto right_s = s+7;
    auto first = std::begin(s);
    auto last = std::end(s) - 1;

    RED_CHECK_EQUAL(ltrim(first, last), left_s);
    RED_CHECK_EQUAL(rtrim(first, last), right_s);

    using range_type = range<char const *>;
    range_type trimmed{left_s, right_s};
    range_type r{first, last};

    RED_CHECK(trim(first, last) == trimmed);
    RED_CHECK(trim(r) == trimmed);
}

RED_AUTO_TEST_CASE(Test_str_concat)
{
    char const* cstr = "abc\0yyy";
    std::string s = "a";
    RED_CHECK_EQ(str_concat("a", "b", "c", "d"), "abcd");
    RED_CHECK_EQ(str_concat("a", "b", "c\0xxx", "d"), "abcd");
    RED_CHECK_EQ(str_concat('a', "b", 'c', "d"), "abcd");
    RED_CHECK_EQ(str_concat("abc", "d"), "abcd");
    RED_CHECK_EQ(str_concat('a', "bc", "d"), "abcd");
    RED_CHECK_EQ(str_concat('a', "bc"_av, std::string("d")), "abcd");
    RED_CHECK_EQ(str_concat(s, "bc"_av, std::string("d")), "abcd");
    RED_CHECK_EQ(str_concat(std::as_const(s), "bc"_av, std::string("d")), "abcd");
    RED_CHECK_EQ(str_concat(std::move(s), "bc"_av, std::string("d")), "abcd");
    RED_CHECK_EQ(str_concat(cstr, "d"), "abcd");
}

RED_AUTO_TEST_CASE(Test_str_append)
{
    char const* cstr = "fg";
    std::string s = "a";
    str_append(s, "b\0xxx", "cd"_av, "e", cstr, std::string("h"), 'i');
    RED_CHECK_EQ(s, "abcdefghi");
}

RED_AUTO_TEST_CASE(Test_str_assign)
{
    char const* cstr = "fg";
    std::string s = "a";
    str_assign(s, "b\0xxx", "cd"_av, "e", cstr, std::string("h"), 'i');
    RED_CHECK_EQ(s, "bcdefghi");
}
