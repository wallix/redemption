/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/strutils.hpp"
#include <string_view>
#include <cstring>

using namespace std::string_view_literals;

RED_AUTO_TEST_CASE(TestEndsWith)
{
    RED_CHECK(!utils::ends_with(""sv, "ini"sv));
    RED_CHECK(!utils::ends_with("ni"sv, "ini"sv));
    RED_CHECK(!utils::ends_with("init"sv, "ini"sv));
    RED_CHECK(!utils::ends_with("rdpproxy.conf"sv, "ini"sv));
    RED_CHECK(!utils::ends_with("Ini"sv, "ini"sv));
    RED_CHECK(!utils::ends_with("RDPPROXY.INI"sv, "ini"sv));

    RED_CHECK(utils::ends_with("ini"sv, "ini"sv));
    RED_CHECK(utils::ends_with(".ini"sv, "ini"sv));
    RED_CHECK(utils::ends_with("rdpproxy.ini"sv, "ini"sv));

    RED_CHECK(utils::ends_with("RDPPROXY.INI"sv, ""sv));
    RED_CHECK(utils::ends_with(""sv, ""sv));
}

RED_AUTO_TEST_CASE(TestStartsWith)
{
    RED_CHECK(!utils::starts_with(""sv, "ini"sv));
    RED_CHECK(!utils::starts_with("ni"sv, "ini"sv));
    RED_CHECK(!utils::starts_with("rdpproxy.conf"sv, "ini"sv));
    RED_CHECK(!utils::starts_with("Ini"sv, "ini"sv));
    RED_CHECK(!utils::starts_with(".ini"sv, "ini"sv));

    RED_CHECK(utils::starts_with("init"sv, "ini"sv));
    RED_CHECK(utils::starts_with("ini"sv, "ini"sv));
    RED_CHECK(utils::starts_with("init"sv, "i"sv));
    RED_CHECK(utils::starts_with(""sv, ""sv));
}

RED_AUTO_TEST_CASE(TestEndsCaseWith)
{
    RED_CHECK(!utils::ends_case_with(""sv, "ini"sv));
    RED_CHECK(!utils::ends_case_with("ni"sv, "ini"sv));
    RED_CHECK(!utils::ends_case_with("NI"sv, "ini"sv));
    RED_CHECK(!utils::ends_case_with("init"sv, "ini"sv));
    RED_CHECK(!utils::ends_case_with("INIT"sv, "ini"sv));
    RED_CHECK(!utils::ends_case_with("rdpproxy.conf"sv, "ini"sv));
    RED_CHECK(!utils::ends_case_with("RDPPROXY.CONF"sv, "ini"sv));

    RED_CHECK(utils::ends_case_with("ini"sv, "ini"sv));
    RED_CHECK(utils::ends_case_with("Ini"sv, "ini"sv));
    RED_CHECK(utils::ends_case_with(".ini"sv, "ini"sv));
    RED_CHECK(utils::ends_case_with(".INI"sv, "ini"sv));
    RED_CHECK(utils::ends_case_with("rdpproxy.ini"sv, "ini"sv));
    RED_CHECK(utils::ends_case_with("RDPPROXY.INI"sv, "ini"sv));

    RED_CHECK(utils::ends_case_with("RDPPROXY.INI"sv, ""sv));
    RED_CHECK(utils::ends_case_with(""sv, ""sv));
}

RED_AUTO_TEST_CASE(Teststrlcpy)
{
    using utils::strlcpy;
    char dest[10]{};
    RED_CHECK(0u == strlcpy(dest, "", 0));
    RED_CHECK(0u == strlcpy(dest, "", 1));
    RED_CHECK(1u == strlcpy(dest, "a"_av, 0));
    RED_CHECK(dest[0] == '\0');
    RED_CHECK(1u == strlcpy(dest, "a"_av, 2));
    RED_CHECK(dest == "a"sv);
    RED_CHECK(7u == strlcpy(dest, "abcdefg"_av, 5));
    RED_CHECK(dest[4] == 0);
}

RED_AUTO_TEST_CASE(Teststrbcpy)
{
    using utils::strbcpy;
    char dest[10]{};
    RED_CHECK(!strbcpy(dest, "", 0));
    RED_CHECK(strbcpy(dest, "", 1));
    RED_CHECK(!strbcpy(dest, "a"_av, 0));
    RED_CHECK(strbcpy(dest, "a"_av, 2));
    RED_CHECK(!strbcpy(dest, "abcdefg"_av, 5));
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

RED_AUTO_TEST_CASE(TestTrim)
{
    char s[] = " \t abcd   ";
    auto av = make_writable_array_view(s).drop_back(1);

    RED_CHECK_EQUAL(ltrim(chars_view(av).last(0)), ""_av);
    RED_CHECK_EQUAL(ltrim(chars_view(av).last(3)), ""_av);
    RED_CHECK_EQUAL(rtrim(chars_view(av).last(0)), ""_av);
    RED_CHECK_EQUAL(rtrim(chars_view(av).last(3)), ""_av);
    RED_CHECK_EQUAL(ltrim(chars_view(av)), "abcd   "_av);
    RED_CHECK_EQUAL(rtrim(chars_view(av)), " \t abcd"_av);
    RED_CHECK_EQUAL(trim(chars_view(av)), "abcd"_av);

    RED_CHECK_EQUAL(ltrim(av.last(0)), ""_av);
    RED_CHECK_EQUAL(ltrim(av.last(3)), ""_av);
    RED_CHECK_EQUAL(rtrim(av.last(0)), ""_av);
    RED_CHECK_EQUAL(rtrim(av.last(3)), ""_av);
    RED_CHECK_EQUAL(ltrim(av), "abcd   "_av);
    RED_CHECK_EQUAL(rtrim(av), " \t abcd"_av);
    RED_CHECK_EQUAL(trim(av), "abcd"_av);
}
