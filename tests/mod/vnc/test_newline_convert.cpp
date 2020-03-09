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

#include "mod/vnc/newline_convert.hpp"
#include <string_view>

using namespace std::string_view_literals;

RED_AUTO_TEST_CASE(TestInPlaceWindowsToLinuxNewLineConverter0)
{
    char rawbuf[32];
    auto buf = make_array_view(rawbuf);

#define TEST(s, r) do {                                                                 \
    std::string in = s; /*NOLINT*/                                                      \
    std::string in2 = s; /*NOLINT*/                                                     \
    auto expected = r ""_av;                                                            \
    auto minilen = std::min(expected.size(), size_t(4));                                \
    RED_CHECK(windows_to_linux_newline_convert(in, buf) == expected);                   \
    RED_CHECK(windows_to_linux_newline_convert(in, buf.first(4)) ==                     \
        expected.first(minilen));                                                       \
    RED_CHECK(windows_to_linux_newline_convert(in, in) == expected);                    \
    RED_CHECK(windows_to_linux_newline_convert(in2, array_view(in2.data(), minilen)) == \
        expected.first(minilen));                                                       \
} while(0)

    TEST("", "");
    TEST("\r\r", "\r\r");
    TEST("\n\n", "\n\n");
    TEST("\r\r\r\r", "\r\r\r\r");
    TEST("\n\n\n\n", "\n\n\n\n");
    TEST("\n\n\r\n", "\n\n\n");
    TEST("\n\r\n\n", "\n\n\n");
    TEST("\r\r\n\r", "\r\n\r");
    TEST("\r\n\r\r", "\n\r\r");
    TEST("\r \n", "\r \n");
    TEST("\r\r \n", "\r\r \n");
    TEST("\n \r", "\n \r");
    TEST("\n\n \r", "\n\n \r");
    TEST("\r\n", "\n");
    TEST("\r\n\r\n", "\n\n");
    TEST("\r\r\n", "\r\n");
    TEST("\n\r\n", "\n\n");
    TEST("\r\ntoto", "\ntoto");
    TEST("\r\nto\r\nto", "\nto\nto");
    TEST("\r\nto\r\nto\r\n", "\nto\nto\n");
    TEST("\r\nto\r\nto\r\n!", "\nto\nto\n!");
    TEST("toto", "toto");
    TEST("toto\r\n", "toto\n");
    TEST("to\r\nto", "to\nto");
    TEST("to\r\nto\r\n", "to\nto\n");
    TEST("to\r\nto\r\n!", "to\nto\n!");

#undef TEST
}

//RED_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter0)
//{
//    const bool cancel_if_buffer_too_small = false;
//
//    size_t result_size;
//
//    {
//        char   s[5] = "toto";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto"sv);
//        RED_CHECK(result_size == 5);
//    }
//
//    {
//        char s[7] = "toto\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto\r\n"sv);
//        RED_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "\ntoto";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\ntoto"sv);
//        RED_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "to\nto";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "to\r\nto"sv);
//        RED_CHECK(result_size == 7);
//    }
//
//    {
//        char s[9] = "\nto\nto";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\nto\r\nto"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[11] = "\nto\nto\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\nto\r\nto\r\n"sv);
//        RED_CHECK(result_size == 11);
//    }
//
//    {
//        char s[17] = "\n\nto\n\nto\n\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\n\r\nto\r\n\r\nto\r\n\r\n"sv);
//        RED_CHECK(result_size == 17);
//    }
//}

//RED_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter1)
//{
//    const bool cancel_if_buffer_too_small = false;
//
//    size_t result_size;
//
//    {
//        char s[6] = "toto\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto"sv);
//        RED_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "toto\n\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto\r\n"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[8] = "toto\n\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto\r\n"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "toto\n\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto\r\n\r\n"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[10] = "toto\n\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto\r\n\r\n"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\n\ntoto";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\n\r\nto"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "\n\ntoto";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\n\r\ntoto"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\ntoto\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\ntoto"sv);
//        RED_CHECK(result_size == 9);
//    }
//}

//RED_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter2)
//{
//    const bool cancel_if_buffer_too_small = true;
//
//    size_t result_size;
//
//    {
//        char s[6] = "toto\n";
//        RED_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        RED_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "toto\n\n";
//        RED_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[8] = "toto\n\n";
//        RED_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "toto\n\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto\r\n\r\n"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[10] = "toto\n\n";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "toto\r\n\r\n"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\n\ntoto";
//        RED_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "\n\ntoto";
//        RED_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s);
//        RED_CHECK(s == "\r\n\r\ntoto"sv);
//        RED_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\ntoto\n";
//        RED_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        RED_CHECK(result_size == 9);
//    }
//}

RED_AUTO_TEST_CASE(TestLinuxToWindowsNewLineConverter)
{
    //
    // RED_CHECK(linux_to_windows_newline_convert(nullptr, 0, nullptr, 0) == 0);
    //
    // {
    //     char d[8];
    //     RED_CHECK(linux_to_windows_newline_convert(nullptr, 0, make_array_view(d)) == 0);
    // }


    {
        char d[8];
        RED_CHECK(linux_to_windows_newline_convert("text"_av.first(0), make_array_view(d)) == ""sv);
    }

    {
        RED_CHECK_EXCEPTION_ERROR_ID(
            linux_to_windows_newline_convert("text"sv, nullptr),
            ERR_STREAM_MEMORY_TOO_SMALL
        );
    }

    {
        char d[2];
        RED_CHECK_EXCEPTION_ERROR_ID(
            linux_to_windows_newline_convert("text"sv, make_array_view(d)),
            ERR_STREAM_MEMORY_TOO_SMALL
        );
    }

    {
        char d[8];
        RED_CHECK(linux_to_windows_newline_convert("\0"sv, make_array_view(d)) == "\0"sv);
    }

    {
        char d[8];
        RED_CHECK(linux_to_windows_newline_convert("toto"sv, make_array_view(d)) == "toto"sv);
    }

    {
        char d[8];
        RED_CHECK(linux_to_windows_newline_convert("toto\n"sv, make_array_view(d)) == "toto\r\n"sv);
    }

    {
        char d[8];
        RED_CHECK(linux_to_windows_newline_convert("toto\n\0"sv, make_array_view(d)) == "toto\r\n\0"sv);
    }

    {
        char d[8];
        RED_CHECK(linux_to_windows_newline_convert("\ntoto"sv, make_array_view(d)) == "\r\ntoto"sv);
    }

    {
        char d[8];
        RED_CHECK(linux_to_windows_newline_convert("to\nto"sv, make_array_view(d)) == "to\r\nto"sv);
    }

    {
        char d[12];
        RED_CHECK(linux_to_windows_newline_convert("\nto\nto\n"sv, make_array_view(d)) == "\r\nto\r\nto\r\n"sv);
    }
}
