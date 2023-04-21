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

#include "utils/ascii.hpp"


RED_AUTO_TEST_CASE(TestUpper)
{
    RED_CHECK(ascii_to_limited_upper<3>("1a."_av).sv() == "1A."_av);
    RED_CHECK(ascii_to_limited_upper<3>("1a.."_av).sv() == ""_av);
    RED_CHECK(ascii_to_limited_upper<50>("abcdefghijklmnopqrstuvwxyz"_av).sv() == "ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av);
    RED_CHECK(ascii_to_limited_upper<5>("abcdefghijklmnopqrstuvwxyz"_av).sv() == ""_av);
    RED_CHECK(ascii_to_limited_zupper<50>("abcdefghijklmnopqrstuvwxyz"_av).sv() == "ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av);
    RED_CHECK(ascii_to_limited_zupper<5>("abcdefghijklmnopqrstuvwxyz"_av).sv() == ""_av);
    RED_CHECK(ascii_to_limited_zupper<50>("abcdefghijklmnopqrstuvwxyz"_av).zsv()[26] == '\0');
    RED_CHECK("abcdefghijklmnopqrstuvwxyz"_ascii_upper.sv() == "ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av);
    RED_CHECK((ascii_to_limited_upper<50>("abcdefghijklmnopqrstuvwxyz"_av) == "abcdefghijklmnopqrstuvwxyz"_ascii_upper));
    RED_CHECK(ascii_to_limited_upper<100>("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"_av).sv()
        == "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"_av);

    RED_CHECK(insensitive_eq("abC"_av, "abc"_ascii_upper));
    RED_CHECK(insensitive_eq("ABC"_av, "abc"_ascii_upper));
    RED_CHECK(insensitive_eq("abc"_av, "abc"_ascii_upper));
    RED_CHECK(!insensitive_eq("axc"_av, "abc"_ascii_upper));
    RED_CHECK(!insensitive_eq("ab"_av, "abc"_ascii_upper));

    RED_CHECK(ascii_is_upper('A'));
    RED_CHECK(ascii_is_upper('Z'));
    RED_CHECK(!ascii_is_upper('a'));
    RED_CHECK(!ascii_is_upper('z'));
    RED_CHECK(!ascii_is_upper('0'));
    RED_CHECK(!ascii_is_upper('"'));

    RED_CHECK(ascii_contains_upper("AA'Z0"_av));
    RED_CHECK(ascii_contains_upper("aa'Z0"_av));
    RED_CHECK(!ascii_contains_upper("aa'z0"_av));
}

RED_AUTO_TEST_CASE(TestLower)
{
    RED_CHECK(ascii_to_limited_lower<3>("1A."_av).sv() == "1a."_av);
    RED_CHECK(ascii_to_limited_lower<3>("1A.."_av).sv() == ""_av);
    RED_CHECK(ascii_to_limited_lower<50>("ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av).sv() == "abcdefghijklmnopqrstuvwxyz"_av);
    RED_CHECK(ascii_to_limited_lower<5>("ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av).sv() == ""_av);
    RED_CHECK(ascii_to_limited_zlower<50>("ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av).sv() == "abcdefghijklmnopqrstuvwxyz"_av);
    RED_CHECK(ascii_to_limited_zlower<5>("ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av).sv() == ""_av);
    RED_CHECK(ascii_to_limited_zlower<50>("ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av).zsv()[26] == '\0');
    RED_CHECK("ABCDEFGHIJKLMNOPQRSTUVWXYZ"_ascii_lower.sv() == "abcdefghijklmnopqrstuvwxyz"_av);
    RED_CHECK((ascii_to_limited_lower<50>("ABCDEFGHIJKLMNOPQRSTUVWXYZ"_av) == "abcdefghijklmnopqrstuvwxyz"_ascii_lower));
    RED_CHECK(ascii_to_limited_lower<100>("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"_av).sv()
        == "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~0123456789abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"_av);

    RED_CHECK(insensitive_eq("abC"_av, "abc"_ascii_lower));
    RED_CHECK(insensitive_eq("ABC"_av, "abc"_ascii_lower));
    RED_CHECK(insensitive_eq("abc"_av, "abc"_ascii_lower));
    RED_CHECK(!insensitive_eq("axc"_av, "abc"_ascii_lower));
    RED_CHECK(!insensitive_eq("ab"_av, "abc"_ascii_lower));

    RED_CHECK(ascii_is_lower('a'));
    RED_CHECK(ascii_is_lower('z'));
    RED_CHECK(!ascii_is_lower('0'));
    RED_CHECK(!ascii_is_lower('"'));
    RED_CHECK(!ascii_is_lower('A'));
    RED_CHECK(!ascii_is_lower('Z'));

    RED_CHECK(ascii_contains_lower("aa'z0"_av));
    RED_CHECK(ascii_contains_lower("aa'Z0"_av));
    RED_CHECK(!ascii_contains_lower("AA'Z0"_av));
}

RED_AUTO_TEST_CASE(TestStartsWith)
{
    using namespace std::string_view_literals;

    using StrUpper = TaggedStringView<UpperTag>;
    RED_CHECK(!StrUpper{"ABC"}.starts_with(StrUpper{"B"}));
    RED_CHECK(!StrUpper{"ABC"}.starts_with(StrUpper{"BCD"}));
    RED_CHECK(!StrUpper{"ABC"}.starts_with(StrUpper{"ABCD"}));
    RED_CHECK(StrUpper{"ABC"}.starts_with(StrUpper{std::string_view("ABCD", 3)}));
    RED_CHECK(StrUpper{"ABC"}.starts_with(StrUpper{std::string_view("ABCD", 2)}));
    RED_CHECK(StrUpper{"ABC"}.starts_with(StrUpper{std::string_view("ABCD", 1)}));
    RED_CHECK(StrUpper{"ABC"}.starts_with(StrUpper{std::string_view("ABCD", 0)}));
    RED_CHECK(StrUpper{"ABC"}.starts_with(StrUpper{}));
}

RED_AUTO_TEST_CASE(TestEndsWith)
{
    using namespace std::string_view_literals;

    using StrUpper = TaggedStringView<UpperTag>;
    RED_CHECK(!StrUpper{"ABC"}.ends_with(StrUpper{"A"}));
    RED_CHECK(!StrUpper{"ABC"}.ends_with(StrUpper{"ABCD"}));
    RED_CHECK(!StrUpper{"ABC"}.ends_with(StrUpper{"xBC"}));
    RED_CHECK(StrUpper{"ABC"}.ends_with(StrUpper{"ABC"}));
    RED_CHECK(StrUpper{"ABC"}.ends_with(StrUpper{"BC"}));
    RED_CHECK(StrUpper{"ABC"}.ends_with(StrUpper{"C"}));
    RED_CHECK(StrUpper{"ABC"}.ends_with(StrUpper{""}));
}

RED_AUTO_TEST_CASE(TestInsensitiveEq)
{
    RED_CHECK(insensitive_eq("abc"_av, "abc"_ascii_lower));
    RED_CHECK(insensitive_eq("abc"_av, "abc"_ascii_upper));
    RED_CHECK(insensitive_eq("ABC"_av, "abc"_ascii_lower));
    RED_CHECK(insensitive_eq("ABC"_av, "abc"_ascii_upper));
    RED_CHECK(!insensitive_eq("xabc"_av, "abc"_ascii_lower));
    RED_CHECK(!insensitive_eq("xabc"_av, "abc"_ascii_upper));
    RED_CHECK(!insensitive_eq("xABC"_av, "abc"_ascii_lower));
    RED_CHECK(!insensitive_eq("xABC"_av, "abc"_ascii_upper));
    RED_CHECK(!insensitive_eq("abc"_av, "abcx"_ascii_lower));
    RED_CHECK(!insensitive_eq("abc"_av, "abcx"_ascii_upper));
    RED_CHECK(!insensitive_eq("ABC"_av, "abcx"_ascii_lower));
    RED_CHECK(!insensitive_eq("ABC"_av, "abcx"_ascii_upper));
}

RED_AUTO_TEST_CASE(TestInsensitiveStartsWith)
{
    RED_CHECK(!insensitive_starts_with("abc"_av, "abcd"_ascii_lower));
    RED_CHECK(!insensitive_starts_with("abc"_av, "abd"_ascii_lower));
    RED_CHECK(!insensitive_starts_with("abc"_av, "ad"_ascii_lower));
    RED_CHECK(insensitive_starts_with("abc"_av, "abc"_ascii_lower));
    RED_CHECK(insensitive_starts_with("abc"_av, "ab"_ascii_lower));
    RED_CHECK(insensitive_starts_with("abc"_av, "a"_ascii_lower));
    RED_CHECK(insensitive_starts_with("abc"_av, ""_ascii_lower));

    RED_CHECK(!insensitive_starts_with("abc"_av, "abcd"_ascii_upper));
    RED_CHECK(!insensitive_starts_with("abc"_av, "abd"_ascii_upper));
    RED_CHECK(!insensitive_starts_with("abc"_av, "ad"_ascii_upper));
    RED_CHECK(insensitive_starts_with("abc"_av, "abc"_ascii_upper));
    RED_CHECK(insensitive_starts_with("abc"_av, "ab"_ascii_upper));
    RED_CHECK(insensitive_starts_with("abc"_av, "a"_ascii_upper));
    RED_CHECK(insensitive_starts_with("abc"_av, ""_ascii_upper));
}

RED_AUTO_TEST_CASE(TestInsensitiveEndsWith)
{
    RED_CHECK(!insensitive_ends_with("abc"_av, "aabc"_ascii_lower));
    RED_CHECK(!insensitive_ends_with("abc"_av, "bbc"_ascii_lower));
    RED_CHECK(!insensitive_ends_with("abc"_av, "a"_ascii_lower));
    RED_CHECK(!insensitive_ends_with("abc"_av, "ab"_ascii_lower));
    RED_CHECK(insensitive_ends_with("abc"_av, "abc"_ascii_lower));
    RED_CHECK(insensitive_ends_with("abc"_av, "bc"_ascii_lower));
    RED_CHECK(insensitive_ends_with("abc"_av, "c"_ascii_lower));
    RED_CHECK(insensitive_ends_with("abc"_av, ""_ascii_lower));

    RED_CHECK(!insensitive_ends_with("abc"_av, "aabc"_ascii_upper));
    RED_CHECK(!insensitive_ends_with("abc"_av, "bbc"_ascii_upper));
    RED_CHECK(!insensitive_ends_with("abc"_av, "a"_ascii_upper));
    RED_CHECK(!insensitive_ends_with("abc"_av, "ab"_ascii_upper));
    RED_CHECK(insensitive_ends_with("abc"_av, "abc"_ascii_upper));
    RED_CHECK(insensitive_ends_with("abc"_av, "bc"_ascii_upper));
    RED_CHECK(insensitive_ends_with("abc"_av, "c"_ascii_upper));
    RED_CHECK(insensitive_ends_with("abc"_av, ""_ascii_upper));
}


std::true_type to_tagged_string_view(TaggedStringView<UpperTag>);

// check if convertible
static_assert(decltype(to_tagged_string_view(TaggedStringView<UpperTag>()))());
static_assert(decltype(to_tagged_string_view(TaggedString<UpperTag, std::string>()))());
static_assert(decltype(to_tagged_string_view(TaggedStringArray<UpperTag, 16>()))());
static_assert(decltype(to_tagged_string_view(TaggedZStringArray<UpperTag, 16>()))());
