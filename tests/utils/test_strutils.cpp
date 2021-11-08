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

RED_AUTO_TEST_CASE(Test_replace_substr_on_tag)
{
    constexpr auto REPLACEMENT = "|raw_word|"sv;
    constexpr auto TAG = "${TAG}"sv;

    {
        std::string str = "${TAG}";
        std::string expected_str = "|raw_word|";

        utils::replace_substr_on_tag(str, TAG, REPLACEMENT);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG} AAAA ${TAG}${TAG} BBBB ${TAG}";
        std::string expected_str =
            "|raw_word| AAAA |raw_word||raw_word| BBBB |raw_word|";

        utils::replace_substr_on_tag(str, TAG, REPLACEMENT);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "AAAA BBBB CCCC";
        std::string expected_str = str;

        utils::replace_substr_on_tag(str, TAG, REPLACEMENT);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "AAAA${TAG}BBBB";
        std::string expected_str = "AAAABBBB";

        utils::replace_substr_on_tag(str, TAG, "");

        RED_CHECK_EQ(str, expected_str);
    }
}

RED_AUTO_TEST_CASE(Test_replace_substr_on_tag_WhileDecorating)
{
    constexpr auto REPLACEMENT = "|raw_word|"sv;
    constexpr auto TAG = "${TAG}"sv;
    constexpr auto DECORATOR =  "&?"sv;

    {
        std::string str = "${TAG}";
        std::string expected_str = "&?|raw_word|&?";

        utils::replace_substr_on_tag(str, TAG, REPLACEMENT, DECORATOR);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG}${TAG} AAAA ${TAG} BBBB ${TAG}${TAG}";
        std::string expected_str =
            "&?|raw_word|&?&?|raw_word|&? AAAA &?|raw_word|&?"
            " BBBB &?|raw_word|&?&?|raw_word|&?";

        utils::replace_substr_on_tag(str, TAG, REPLACEMENT, DECORATOR);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "123456";
        std::string expected_str = str;

        utils::replace_substr_on_tag(str, TAG, REPLACEMENT, DECORATOR);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "AAAA${TAG}BBBB";
        std::string expected_str = "AAAA&?&?BBBB";

        utils::replace_substr_on_tag(str, TAG, "", DECORATOR);

        RED_CHECK_EQ(str, expected_str);
    }
}

RED_AUTO_TEST_CASE(Test_replace_substr_between_tags)
{
    constexpr auto REPLACEMENT = "********"sv;
    constexpr auto OPENING_TAG = "${TAG_B}"sv;
    constexpr auto CLOSURE_TAG = "${TAG_E}"sv;

    {
        std::string str = "AAAAA ${TAG_B}BBBBB${TAG_E} CCCCC DDDDD";
        std::string expected_str = "AAAAA ******** CCCCC DDDDD";

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "A${TAG_B}a${TAG_E}B${TAG_B}b${TAG_E}C";
        std::string expected_str = "A********B********C";

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}${TAG_E}";
        std::string expected_str = "********";

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}${TAG_E}${TAG_B}ABCDEFGHIJK${TAG_E}";
        std::string expected_str = "****************";

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "AZERTYUIOP1234";
        std::string expected_str = "AZERTYUIOP1234";

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str =
            "${TAG_B}${TAG_B}${TAG_E}1${TAG_B}${TAG_B}${TAG_E}";
        std::string expected_str = "********1********";

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = " ${TAG_B}abcdef${TAG_E} ";
        std::string expected_str = "  ";

        utils::replace_substr_between_tags(str, "", OPENING_TAG, CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str =
            "AAAAA $$$$$$${TAG_B}}BBBBB$$$$$$$${TAG_B}} CCCCC DDDDD";
        std::string expected_str = "AAAAA $$$$$$********} CCCCC DDDDD";

        utils::replace_substr_between_tags(str, REPLACEMENT, OPENING_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str =
            "${TAG_B}1${TAG_B}${TAG_B}2${TAG_B}${TAG_B}${TAG_B}";
        std::string expected_str = "************************";

        utils::replace_substr_between_tags(str, REPLACEMENT, OPENING_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}{TAG_B}${TAG_B}";
        std::string expected_str = "********";

        utils::replace_substr_between_tags(str, REPLACEMENT, OPENING_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = " ${TAG_B}abcdef${TAG_B} ";
        std::string expected_str = "  ";

        utils::replace_substr_between_tags(str, "", OPENING_TAG);

        RED_CHECK_EQ(str, expected_str);
    }



    {
        std::string str = "${TAG_E}1234567890${TAG_B}";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B} ${TAG_E} ${TAG_B} 42";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str =
            "${TAG_B}AAAAAA${TAG_E}BBBBB${TAG_E}CCCCCCC${TAG_B}";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}${TAG_B}4242${TAG_E}${TAG_E}";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}1${TAG_E}2${TAG_B}${TAG_E}${TAG_B}";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str,
                                           REPLACEMENT,
                                           OPENING_TAG,
                                           CLOSURE_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}${TAG_B}${TAG_B}aaa${TAG_B}${TAG_B}";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str, REPLACEMENT, OPENING_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}${TAG_B}${TAG_B}";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str, REPLACEMENT, OPENING_TAG);

        RED_CHECK_EQ(str, expected_str);
    }

    {
        std::string str = "${TAG_B}{TAG_B}$";
        std::string expected_str = str;

        utils::replace_substr_between_tags(str, REPLACEMENT, OPENING_TAG);

        RED_CHECK_EQ(str, expected_str);
    }
}
