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

#define RED_TEST_MODULE TestStrUtils
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/strutils.hpp"


inline bool ends_with(char const * s1, char const * s2) {
    return utils::ends_with(s1, strlen(s1), s2, strlen(s2));
}

inline bool ends_case_with(char const * s1, char const * s2) {
    return utils::ends_case_with(s1, strlen(s1), s2, strlen(s2));
}

RED_AUTO_TEST_CASE(TestEndsWith)
{
    RED_CHECK(!ends_with("", "ini"));
    RED_CHECK(!ends_with("ni", "ini"));
    RED_CHECK(!ends_case_with("init", "ini"));
    RED_CHECK(!ends_with("rdpproxy.conf", "ini"));
    RED_CHECK(!ends_with("Ini", "ini"));
    RED_CHECK(!ends_with("RDPPROXY.INI", "ini"));

    RED_CHECK(ends_with("ini", "ini"));
    RED_CHECK(ends_with(".ini", "ini"));
    RED_CHECK(ends_with("rdpproxy.ini", "ini"));

    RED_CHECK(ends_with("RDPPROXY.INI", ""));
    RED_CHECK(ends_with("", ""));
}

RED_AUTO_TEST_CASE(TestEndsCaseWith)
{
    RED_CHECK(!ends_case_with("", "ini"));
    RED_CHECK(!ends_case_with("ni", "ini"));
    RED_CHECK(!ends_case_with("NI", "ini"));
    RED_CHECK(!ends_case_with("init", "ini"));
    RED_CHECK(!ends_case_with("INIT", "ini"));
    RED_CHECK(!ends_case_with("rdpproxy.conf", "ini"));
    RED_CHECK(!ends_case_with("RDPPROXY.CONF", "ini"));

    RED_CHECK(ends_case_with("ini", "ini"));
    RED_CHECK(ends_case_with("Ini", "ini"));
    RED_CHECK(ends_case_with(".ini", "ini"));
    RED_CHECK(ends_case_with(".INI", "ini"));
    RED_CHECK(ends_case_with("rdpproxy.ini", "ini"));
    RED_CHECK(ends_case_with("RDPPROXY.INI", "ini"));

    RED_CHECK(ends_case_with("RDPPROXY.INI", ""));
    RED_CHECK(ends_case_with("", ""));
}

RED_AUTO_TEST_CASE(Teststrlcpy)
{
    using utils::strlcpy;
    char dest[10]{};
    RED_CHECK_EQ(0u, strlcpy(dest, "", 0));
    RED_CHECK_EQ(0u, strlcpy(dest, "", 1));
    RED_CHECK_EQ(1u, strlcpy(dest, "a"_av, 0));
    RED_CHECK_EQ(dest[0], '\0');
    RED_CHECK_EQ(1u, strlcpy(dest, "a"_av, 2));
    RED_CHECK_EQ(dest, "a");
    RED_CHECK_EQ(7u, strlcpy(dest, "abcdefg"_av, 5));
    RED_CHECK_EQ(dest[4], 0);
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
