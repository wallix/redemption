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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/compare_collection.hpp"

#include "utils/sugar/chars_to_int.hpp"


#if !REDEMPTION_UNIT_TEST_FAST_CHECK
template<class T>
struct RED_TEST_PRINT_TYPE_STRUCT_NAME<parsed_chars_to_int_result<T>>
{
    void operator()(
        REDEMPTION_UT_UNUSED_STREAM std::ostream& out,
        parsed_chars_to_int_result<T> const& value
    ) const
    {
        if (value) {
            REDEMPTION_UT_OSTREAM_PLACEHOLDER(out) << +value.value;
        }
        else {
            REDEMPTION_UT_OSTREAM_PLACEHOLDER(out) << "empty";
        }
    }
};

template<class T>
struct RED_TEST_PRINT_TYPE_STRUCT_NAME<chars_to_int_result<T>>
{
    void operator()(
        REDEMPTION_UT_UNUSED_STREAM std::ostream& out,
        chars_to_int_result<T> const& value
    ) const
    {
        REDEMPTION_UT_OSTREAM_PLACEHOLDER(out) << "{" << value.ec << ", " << +value.val << ", \"" << value.ptr << "\"}";
    }
};

// std::from_chars_result == std::from_chars_result only from C++20
static ut::assertion_result from_chars_result_EQ(std::from_chars_result a, std::from_chars_result b)
{
    return ut::create_assertion_result(
        (a.ptr == b.ptr && a.ec == b.ec), a, " != ", b,
        [](std::ostream& out, std::from_chars_result r){
            REDEMPTION_UT_OSTREAM_PLACEHOLDER(out) << "{" << r.ec << ", " << r.ptr << "}";
        }
    );
}

RED_TEST_DISPATCH_COMPARISON_EQ((), (std::from_chars_result), (std::from_chars_result), ::from_chars_result_EQ)
#endif

RED_AUTO_TEST_CASE(TestParseDecimalChars)
{
    // char*
    RED_CHECK(parse_decimal_chars<int>("") == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("-") == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("x") == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("1x") == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("0") == (parsed_chars_to_int_result<int>{0, true}));
    RED_CHECK(parse_decimal_chars<int>("1") == (parsed_chars_to_int_result<int>{1, true}));
    RED_CHECK(parse_decimal_chars<int>("-0") == (parsed_chars_to_int_result<int>{-0, true}));
    RED_CHECK(parse_decimal_chars<int>("-1") == (parsed_chars_to_int_result<int>{-1, true}));
    RED_CHECK(parse_decimal_chars<int>("0083351") == (parsed_chars_to_int_result<int>{83351, true}));
    RED_CHECK(parse_decimal_chars<int>("-0083351") == (parsed_chars_to_int_result<int>{-83351, true}));
    RED_CHECK(parse_decimal_chars<int>("83351") == (parsed_chars_to_int_result<int>{83351, true}));
    RED_CHECK(parse_decimal_chars<int>("-83351") == (parsed_chars_to_int_result<int>{-83351, true}));
    RED_CHECK(parse_decimal_chars<uint32_t>("4294967295") == (parsed_chars_to_int_result<uint32_t>{4294967295, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("2147483646") == (parsed_chars_to_int_result<int32_t>{2147483646, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("2147483647") == (parsed_chars_to_int_result<int32_t>{2147483647, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("2147483648") == (parsed_chars_to_int_result<int32_t>{0, false}));
    RED_CHECK(parse_decimal_chars<int32_t>("-2147483647") == (parsed_chars_to_int_result<int32_t>{-2147483647, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("-2147483648") == (parsed_chars_to_int_result<int32_t>{-2147483648, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("-2147483649") == (parsed_chars_to_int_result<int32_t>{0, false}));
    RED_CHECK(parse_decimal_chars<int32_t>("1147483647") == (parsed_chars_to_int_result<int32_t>{1147483647, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("-1147483648") == (parsed_chars_to_int_result<int32_t>{-1147483648, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("1147483649") == (parsed_chars_to_int_result<int32_t>{1147483649, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("-1147483649") == (parsed_chars_to_int_result<int32_t>{-1147483649, true}));
    RED_CHECK(parse_decimal_chars<int>("23786287457654283351") == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("-23786287457654283351") == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("000000000000000000000000000123") == (parsed_chars_to_int_result<int>{123, true}));
    RED_CHECK(parse_decimal_chars<int>("-000000000000000000000000000123") == (parsed_chars_to_int_result<int>{-123, true}));

    // chars_view
    RED_CHECK(parse_decimal_chars<int>(""_av) == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("-"_av) == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("x"_av) == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("1x"_av) == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("0"_av) == (parsed_chars_to_int_result<int>{0, true}));
    RED_CHECK(parse_decimal_chars<int>("1"_av) == (parsed_chars_to_int_result<int>{1, true}));
    RED_CHECK(parse_decimal_chars<int>("-0"_av) == (parsed_chars_to_int_result<int>{-0, true}));
    RED_CHECK(parse_decimal_chars<int>("-1"_av) == (parsed_chars_to_int_result<int>{-1, true}));
    RED_CHECK(parse_decimal_chars<int>("0083351"_av) == (parsed_chars_to_int_result<int>{83351, true}));
    RED_CHECK(parse_decimal_chars<int>("-0083351"_av) == (parsed_chars_to_int_result<int>{-83351, true}));
    RED_CHECK(parse_decimal_chars<int>("83351"_av) == (parsed_chars_to_int_result<int>{83351, true}));
    RED_CHECK(parse_decimal_chars<int>("-83351"_av) == (parsed_chars_to_int_result<int>{-83351, true}));
    RED_CHECK(parse_decimal_chars<uint32_t>("4294967295"_av) == (parsed_chars_to_int_result<uint32_t>{4294967295, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("2147483647"_av) == (parsed_chars_to_int_result<int32_t>{2147483647, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("-2147483648"_av) == (parsed_chars_to_int_result<int32_t>{-2147483648, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("1147483647"_av) == (parsed_chars_to_int_result<int32_t>{1147483647, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("-1147483648"_av) == (parsed_chars_to_int_result<int32_t>{-1147483648, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("1147483649"_av) == (parsed_chars_to_int_result<int32_t>{1147483649, true}));
    RED_CHECK(parse_decimal_chars<int32_t>("-1147483649"_av) == (parsed_chars_to_int_result<int32_t>{-1147483649, true}));
    RED_CHECK(parse_decimal_chars<int>("23786287457654283351"_av) == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("-23786287457654283351"_av) == parsed_chars_to_int_result<int>());
    RED_CHECK(parse_decimal_chars<int>("000000000000000000000000000123"_av) == (parsed_chars_to_int_result<int>{123, true}));
    RED_CHECK(parse_decimal_chars<int>("-000000000000000000000000000123"_av) == (parsed_chars_to_int_result<int>{-123, true}));
}

RED_AUTO_TEST_CASE(TestParseHexadecimalChars)
{
    // char*
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("") == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("-") == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("x") == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("0x1") == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("1x") == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("0") == (parsed_chars_to_int_result<uint32_t>{0, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("1") == (parsed_chars_to_int_result<uint32_t>{1, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("-0") == (parsed_chars_to_int_result<uint32_t>()));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("-1") == (parsed_chars_to_int_result<uint32_t>()));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("83351") == (parsed_chars_to_int_result<uint32_t>{0x83351, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("0083351") == (parsed_chars_to_int_result<uint32_t>{0x83351, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("abCD78") == (parsed_chars_to_int_result<uint32_t>{0xabCD78, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("FFFFFFF") == (parsed_chars_to_int_result<uint32_t>{0xFFFFFFF, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("FFFFFFFF") == (parsed_chars_to_int_result<uint32_t>{0xFFFFFFFF, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("FFFFFFFFF") == (parsed_chars_to_int_result<uint32_t>{}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("123456789") == (parsed_chars_to_int_result<uint32_t>()));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("0000000000000000000123") == (parsed_chars_to_int_result<uint32_t>{0x123, true}));

    // chars_view
    RED_CHECK(parse_hexadecimal_chars<uint32_t>(""_av) == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("-"_av) == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("x"_av) == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("1x"_av) == parsed_chars_to_int_result<uint32_t>());
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("0"_av) == (parsed_chars_to_int_result<uint32_t>{0, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("1"_av) == (parsed_chars_to_int_result<uint32_t>{1, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("-0"_av) == (parsed_chars_to_int_result<uint32_t>()));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("-1"_av) == (parsed_chars_to_int_result<uint32_t>()));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("83351"_av) == (parsed_chars_to_int_result<uint32_t>{0x83351, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("0083351"_av) == (parsed_chars_to_int_result<uint32_t>{0x83351, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("abCD78"_av) == (parsed_chars_to_int_result<uint32_t>{0xabCD78, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("FFFFFFF"_av) == (parsed_chars_to_int_result<uint32_t>{0xFFFFFFF, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("FFFFFFFF"_av) == (parsed_chars_to_int_result<uint32_t>{0xFFFFFFFF, true}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("FFFFFFFFF"_av) == (parsed_chars_to_int_result<uint32_t>{}));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("123456789"_av) == (parsed_chars_to_int_result<uint32_t>()));
    RED_CHECK(parse_hexadecimal_chars<uint32_t>("0000000000000000000123"_av) == (parsed_chars_to_int_result<uint32_t>{0x123, true}));
}

RED_AUTO_TEST_CASE(TestParseDecimalCharsOr)
{
    RED_CHECK(parse_decimal_chars_or<int>("123", 42) == 123);
    RED_CHECK(parse_decimal_chars_or<int>("123x", 42) == 42);
    RED_CHECK(parse_decimal_chars_or<int>("123"_av, 42) == 123);
    RED_CHECK(parse_decimal_chars_or<int>("123x"_av, 42) == 42);
    RED_CHECK(parse_decimal_chars_or<int>("123"_av.first(2), 42) == 12);
    RED_CHECK(parse_decimal_chars_or<int>("123x"_av.first(2), 42) == 12);
}

RED_AUTO_TEST_CASE(TestParseHexadecimalCharsOr)
{
    RED_CHECK(parse_hexadecimal_chars_or<unsigned>("123", 42) == 0x123);
    RED_CHECK(parse_hexadecimal_chars_or<unsigned>("123x", 42) == 42);
    RED_CHECK(parse_hexadecimal_chars_or<unsigned>("123"_av, 42) == 0x123);
    RED_CHECK(parse_hexadecimal_chars_or<unsigned>("123x"_av, 42) == 42);
    RED_CHECK(parse_hexadecimal_chars_or<unsigned>("123"_av.first(2), 42) == 0x12);
    RED_CHECK(parse_hexadecimal_chars_or<unsigned>("123x"_av.first(2), 42) == 0x12);
}

RED_AUTO_TEST_CASE(TestDecimalCharsToInt)
{
    char const* s;
    int i = 42;

    s = "";
    RED_CHECK(decimal_chars_to_int<int>(s) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(decimal_chars_to_int<int>(std::string_view(s)) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, s}));

    RED_CHECK(decimal_chars_to_int(s, i) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(i == 42);
    RED_CHECK(decimal_chars_to_int(std::string_view(s), i) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(i == 42);

    s = "-";
    RED_CHECK(decimal_chars_to_int<int>(s) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(decimal_chars_to_int<int>(std::string_view(s)) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, s}));

    s = "12345";
    RED_CHECK(decimal_chars_to_int<int>(s) == (chars_to_int_result<int>{std::errc(), 12345, s+5}));
    RED_CHECK(decimal_chars_to_int<int>(std::string_view(s)) == (chars_to_int_result<int>{std::errc(), 12345, s+5}));
    RED_CHECK(decimal_chars_to_int(s, i) == (chars_to_int_result<int>{std::errc(), 12345, s+5}));
    RED_CHECK(i == 12345);
    i = 42;
    RED_CHECK(decimal_chars_to_int(std::string_view(s), i) == (chars_to_int_result<int>{std::errc(), 12345, s+5}));
    RED_CHECK(i == 12345);

    s = "-12345";
    RED_CHECK(decimal_chars_to_int<int>(s) == (chars_to_int_result<int>{std::errc(), -12345, s+6}));
    RED_CHECK(decimal_chars_to_int<int>(std::string_view(s)) == (chars_to_int_result<int>{std::errc(), -12345, s+6}));

    s = "12345xabc";
    RED_CHECK(decimal_chars_to_int<int>(s) == (chars_to_int_result<int>{std::errc(), 12345, s+5}));
    RED_CHECK(decimal_chars_to_int<int>(std::string_view(s)) == (chars_to_int_result<int>{std::errc(), 12345, s+5}));

    s = "1234567890987654321";
    RED_CHECK(decimal_chars_to_int<int32_t>(s) == (chars_to_int_result<int32_t>{std::errc::result_out_of_range, 0, s+10}));
    RED_CHECK(decimal_chars_to_int<int32_t>(std::string_view(s)) == (chars_to_int_result<int32_t>{std::errc::result_out_of_range, 0, s+10}));

    s = "2555";
    RED_CHECK(decimal_chars_to_int<uint8_t>(s) == (chars_to_int_result<uint8_t>{std::errc::result_out_of_range, 0, s+3}));
    RED_CHECK(decimal_chars_to_int<uint8_t>(std::string_view(s)) == (chars_to_int_result<uint8_t>{std::errc::result_out_of_range, 0, s+3}));
}

RED_AUTO_TEST_CASE(TestHexadecimalCharsToInt)
{
    char const* s;
    unsigned i = 42;

    s = "";
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(s) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(std::string_view(s)) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(hexadecimal_chars_to_int(s, i) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(i == 42);
    RED_CHECK(hexadecimal_chars_to_int(std::string_view(s), i) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(i == 42);

    s = "-";
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(s) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(std::string_view(s)) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));

    s = "12345";
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(s) == (chars_to_int_result<uint32_t>{std::errc(), 0x12345, s+5}));
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(std::string_view(s)) == (chars_to_int_result<uint32_t>{std::errc(), 0x12345, s+5}));
    RED_CHECK(hexadecimal_chars_to_int(std::string_view(s), i) == (chars_to_int_result<uint32_t>{std::errc(), 0x12345, s+5}));
    RED_CHECK(i == 0x12345);
    i = 42;
    RED_CHECK(hexadecimal_chars_to_int(s, i) == (chars_to_int_result<uint32_t>{std::errc(), 0x12345, s+5}));
    RED_CHECK(i == 0x12345);

    s = "-12345";
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(s) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(std::string_view(s)) == (chars_to_int_result<uint32_t>{std::errc::invalid_argument, 0, s}));

    s = "12345xabc";
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(s) == (chars_to_int_result<uint32_t>{std::errc(), 0x12345, s+5}));
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(std::string_view(s)) == (chars_to_int_result<uint32_t>{std::errc(), 0x12345, s+5}));

    s = "1234567890987654321";
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(s) == (chars_to_int_result<uint32_t>{std::errc::result_out_of_range, 0, s+8}));
    RED_CHECK(hexadecimal_chars_to_int<uint32_t>(std::string_view(s)) == (chars_to_int_result<uint32_t>{std::errc::result_out_of_range, 0, s+8}));
}

RED_AUTO_TEST_CASE(TestFromDecimalChars)
{
    char const* s;
    int i = 42;

    s = "";
    RED_CHECK(from_decimal_chars(s, i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);
    RED_CHECK(from_decimal_chars(std::string_view(s), i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);

    s = "-";
    RED_CHECK(from_decimal_chars(s, i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);
    RED_CHECK(from_decimal_chars(std::string_view(s), i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);

    s = "12345";
    RED_CHECK(from_decimal_chars(s, i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 12345);
    i = 42;
    RED_CHECK(from_decimal_chars(std::string_view(s), i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 12345);
    i = 42;

    s = "-12345";
    RED_CHECK(from_decimal_chars(s, i) == (std::from_chars_result{s+6, std::errc()}));
    RED_CHECK(i == -12345);
    i = 42;
    RED_CHECK(from_decimal_chars(std::string_view(s), i) == (std::from_chars_result{s+6, std::errc()}));
    RED_CHECK(i == -12345);
    i = 42;

    s = "12345xabc";
    RED_CHECK(from_decimal_chars(s, i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 12345);
    i = 42;
    RED_CHECK(from_decimal_chars(std::string_view(s), i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 12345);
    // i = 42;

    uint32_t u = 42;

    s = "1234567890987654321";
    RED_CHECK(from_decimal_chars(s, u) == (std::from_chars_result{s+19, std::errc::result_out_of_range}));
    RED_CHECK(u == 42);
    RED_CHECK(from_decimal_chars(std::string_view(s), u) == (std::from_chars_result{s+19, std::errc::result_out_of_range}));
    RED_CHECK(u == 42);
}

RED_AUTO_TEST_CASE(TestFromHexadecimalChars)
{
    char const* s;
    unsigned i = 42;

    s = "";
    RED_CHECK(from_hexadecimal_chars(s, i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);
    RED_CHECK(from_hexadecimal_chars(std::string_view(s), i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);

    s = "-";
    RED_CHECK(from_hexadecimal_chars(s, i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);
    RED_CHECK(from_hexadecimal_chars(std::string_view(s), i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);

    s = "12345";
    RED_CHECK(from_hexadecimal_chars(s, i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 0x12345);
    i = 42;
    RED_CHECK(from_hexadecimal_chars(std::string_view(s), i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 0x12345);
    i = 42;

    s = "-12345";
    RED_CHECK(from_hexadecimal_chars(s, i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);
    RED_CHECK(from_hexadecimal_chars(std::string_view(s), i) == (std::from_chars_result{s, std::errc::invalid_argument}));
    RED_CHECK(i == 42);

    s = "12345xabc";
    RED_CHECK(from_hexadecimal_chars(s, i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 0x12345);
    i = 42;
    RED_CHECK(from_hexadecimal_chars(std::string_view(s), i) == (std::from_chars_result{s+5, std::errc()}));
    RED_CHECK(i == 0x12345);
    // i = 42;

    uint32_t u = 42;

    s = "1234567890987654321";
    RED_CHECK(from_hexadecimal_chars(s, u) == (std::from_chars_result{s+19, std::errc::result_out_of_range}));
    RED_CHECK(u == 42);
    RED_CHECK(from_hexadecimal_chars(std::string_view(s), u) == (std::from_chars_result{s+19, std::errc::result_out_of_range}));
    RED_CHECK(u == 42);

    s = "1234567890987654321xabc";
    RED_CHECK(from_hexadecimal_chars(s, u) == (std::from_chars_result{s+19, std::errc::result_out_of_range}));
    RED_CHECK(u == 42);
    RED_CHECK(from_hexadecimal_chars(std::string_view(s), u) == (std::from_chars_result{s+19, std::errc::result_out_of_range}));
    RED_CHECK(u == 42);
}

RED_AUTO_TEST_CASE(TestUncheckCharsToInt)
{
    RED_CHECK(int(unchecked_decimal_chars_to_int("123456")) == 123456);
    RED_CHECK(int(unchecked_decimal_chars_to_int("123456"_av)) == 123456);
    RED_CHECK(int(unchecked_decimal_chars_to_int(std::string_view("123456"))) == 123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_without_prefix_to_int("123456")) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_without_prefix_to_int("123456"_av)) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_without_prefix_to_int(std::string_view("123456"))) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int("123456")) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int("123456"_av)) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int(std::string_view("123456"))) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int("0x123456")) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int("0x123456"_av)) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int(std::string_view("0x123456"))) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int("0X123456")) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int("0X123456"_av)) == 0x123456);
    RED_CHECK(unsigned(unchecked_hexadecimal_chars_with_prefix_to_int(std::string_view("0X123456"))) == 0x123456);
}

RED_AUTO_TEST_CASE(TestStringToInt)
{
    auto str_i = "  123354"_av;
    auto str_hex = "  0x23002"_av;
    auto str_neg = "  -123354"_av;
    auto str_neg_hex = "  -0x23002"_av;
    auto str_fake_hex = "  -0xY"_av;
    auto str_i_long = "  111111111111111111111111111111111111111111111111  "_av;
    auto str_hex_long = "  111111111111111111111111111111111111111111111111  "_av;

    RED_CHECK(string_to_int<int>(str_i) == (chars_to_int_result<int>{std::errc(), 123354, str_i.end()}));
    RED_CHECK(string_to_int<int>(str_i.data()) == (chars_to_int_result<int>{std::errc(), 123354, str_i.end()}));

    RED_CHECK(string_to_int<unsigned>(str_i) == (chars_to_int_result<unsigned>{std::errc(), 123354, str_i.end()}));
    RED_CHECK(string_to_int<unsigned>(str_i.data()) == (chars_to_int_result<unsigned>{std::errc(), 123354, str_i.end()}));

    RED_CHECK(string_to_int<int>(str_hex) == (chars_to_int_result<int>{std::errc(), 0x23002, str_hex.end()}));
    RED_CHECK(string_to_int<int>(str_hex.data()) == (chars_to_int_result<int>{std::errc(), 0x23002, str_hex.end()}));

    RED_CHECK(string_to_int<unsigned>(str_hex) == (chars_to_int_result<unsigned>{std::errc(), 0x23002, str_hex.end()}));
    RED_CHECK(string_to_int<unsigned>(str_hex.data()) == (chars_to_int_result<unsigned>{std::errc(), 0x23002, str_hex.end()}));

    RED_CHECK(string_to_int<int>(str_neg) == (chars_to_int_result<int>{std::errc(), -123354, str_neg.end()}));
    RED_CHECK(string_to_int<int>(str_neg.data()) == (chars_to_int_result<int>{std::errc(), -123354, str_neg.end()}));

    RED_CHECK(string_to_int<unsigned>(str_neg) == (chars_to_int_result<unsigned>{std::errc::invalid_argument, 0, str_neg.begin() + 2}));
    RED_CHECK(string_to_int<unsigned>(str_neg.data()) == (chars_to_int_result<unsigned>{std::errc::invalid_argument, 0, str_neg.begin() + 2}));

    RED_CHECK(string_to_int<int>(str_neg_hex) == (chars_to_int_result<int>{std::errc(), -0x23002, str_neg_hex.end()}));
    RED_CHECK(string_to_int<int>(str_neg_hex.data()) == (chars_to_int_result<int>{std::errc(), -0x23002, str_neg_hex.end()}));

    RED_CHECK(string_to_int<unsigned>(str_neg_hex) == (chars_to_int_result<unsigned>{std::errc::invalid_argument, 0, str_neg_hex.begin() + 2}));
    RED_CHECK(string_to_int<unsigned>(str_neg_hex.data()) == (chars_to_int_result<unsigned>{std::errc::invalid_argument, 0, str_neg_hex.begin() + 2}));

    RED_CHECK(string_to_int<int>(str_fake_hex) == (chars_to_int_result<int>{std::errc(), 0, str_fake_hex.begin() + 4}));
    RED_CHECK(string_to_int<int>(str_fake_hex.data()) == (chars_to_int_result<int>{std::errc(), 0, str_fake_hex.begin() + 4}));

    RED_CHECK(string_to_int<unsigned>(str_fake_hex) == (chars_to_int_result<unsigned>{std::errc::invalid_argument, 0, str_fake_hex.begin() + 2}));
    RED_CHECK(string_to_int<unsigned>(str_fake_hex.data()) == (chars_to_int_result<unsigned>{std::errc::invalid_argument, 0, str_fake_hex.begin() + 2}));

    RED_CHECK(string_to_int<int>(str_i_long) == (chars_to_int_result<int>{std::errc::result_out_of_range, 0, str_i_long.end() - 2}));
    RED_CHECK(string_to_int<int>(str_i_long.data()) == (chars_to_int_result<int>{std::errc::result_out_of_range, 0, str_i_long.end() - 2}));

    RED_CHECK(string_to_int<int>(str_hex_long) == (chars_to_int_result<int>{std::errc::result_out_of_range, 0, str_hex_long.end() - 2}));
    RED_CHECK(string_to_int<int>(str_hex_long.data()) == (chars_to_int_result<int>{std::errc::result_out_of_range, 0, str_hex_long.end() - 2}));

    auto str = ""_av;
    RED_CHECK(string_to_int<int>(str) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, str.end()}));
    str = "   "_av;
    RED_CHECK(string_to_int<int>(str) == (chars_to_int_result<int>{std::errc::invalid_argument, 0, str.end()}));

    str = " -123"_av;
    RED_CHECK(string_to_int<unsigned>(str) == (chars_to_int_result<unsigned>{std::errc::invalid_argument, 0, str.begin()+1}));

    str = "-0xffff"_av;
    RED_CHECK(string_to_int<int16_t>(str) == (chars_to_int_result<int16_t>{std::errc::result_out_of_range, 0, str.end()}));

    str = "-0x7fff"_av;
    RED_CHECK(string_to_int<int16_t>(str) == (chars_to_int_result<int16_t>{std::errc(), -0x7fff, str.end()}));

    str = "-0x8000"_av;
    RED_CHECK(string_to_int<int16_t>(str) == (chars_to_int_result<int16_t>{std::errc(), -0x8000, str.end()}));

    str = "-0x8001"_av;
    RED_CHECK(string_to_int<int16_t>(str) == (chars_to_int_result<int16_t>{std::errc::result_out_of_range, 0, str.end()}));
}

RED_AUTO_TEST_CASE(TestCharsToIntFromInitializerList)
{
    unsigned i;
    char const* s = "1";
    decimal_chars_to_int<unsigned>({s, s+1});
    decimal_chars_to_int({s, s+1}, i);
    from_decimal_chars({s, s+1}, i);
    parse_decimal_chars<unsigned>({s, s+1});
    parse_decimal_chars_or({s, s+1}, 0u);

    hexadecimal_chars_to_int<unsigned>({s, s+1});
    hexadecimal_chars_to_int({s, s+1}, i);
    from_hexadecimal_chars({s, s+1}, i);
    parse_hexadecimal_chars<unsigned>({s, s+1});
    parse_hexadecimal_chars_or({s, s+1}, 0u);
}
