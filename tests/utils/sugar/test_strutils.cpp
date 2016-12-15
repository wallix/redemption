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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestStrUtils
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "utils/sugar/strutils.hpp"

BOOST_AUTO_TEST_CASE(TestInPlaceWindowsToLinuxNewLineConverter0)
{
    {
        char s[] = "toto";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "toto"));
    }

    {
        char s[] = "\r\ntoto";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\ntoto"));
    }

    {
        char s[] = "toto\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "toto\n"));
    }

    {
        char s[] = "to\r\nto";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "to\nto"));
    }

    {
        char s[] = "\r\nto\r\nto";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\nto\nto"));
    }

    {
        char s[] = "to\r\nto\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "to\nto\n"));
    }

    {
        char s[] = "\r\nto\r\nto\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\nto\nto\n"));
    }

    {
        char s[] = "to\r\nto\r\n!";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "to\nto\n!"));
    }

    {
        char s[] = "\r\nto\r\nto\r\n!";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\nto\nto\n!"));
    }

    {
        char s[] = "\r\n\r\nto\r\n\r\nto\r\n\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\n\nto\n\nto\n\n"));
    }
}

BOOST_AUTO_TEST_CASE(TestInPlaceWindowsToLinuxNewLineConverter1)
{
    {
        char s[] = "";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, ""));
    }

    {
        char s[] = "\r\r";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, "\r\r"));
    }

    {
        char s[] = "\n\n";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, "\n\n"));
    }

    {
        char s[] = "\r \n";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, "\r \n"));
    }

    {
        char s[] = "\r\n";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, "\n"));
    }

    {
        char s[] = "\r\n\r\n";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, "\n\n"));
    }

    {
        char s[] = "\r\r\n";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, "\r\n"));
    }

    {
        char s[] = "\n\r\n";
        BOOST_CHECK((in_place_windows_to_linux_newline_convert(s) == s) && !strcmp(s, "\n\n"));
    }
}

//BOOST_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter0)
//{
//    const bool cancel_if_buffer_too_small = false;
//
//    size_t result_size;
//
//    {
//        char   s[5] = "toto";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto"));
//        BOOST_CHECK(result_size == 5);
//    }
//
//    {
//        char s[7] = "toto\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto\r\n"));
//        BOOST_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "\ntoto";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\ntoto"));
//        BOOST_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "to\nto";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "to\r\nto"));
//        BOOST_CHECK(result_size == 7);
//    }
//
//    {
//        char s[9] = "\nto\nto";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\nto\r\nto"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[11] = "\nto\nto\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\nto\r\nto\r\n"));
//        BOOST_CHECK(result_size == 11);
//    }
//
//    {
//        char s[17] = "\n\nto\n\nto\n\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\n\r\nto\r\n\r\nto\r\n\r\n"));
//        BOOST_CHECK(result_size == 17);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter1)
//{
//    const bool cancel_if_buffer_too_small = false;
//
//    size_t result_size;
//
//    {
//        char s[6] = "toto\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto"));
//        BOOST_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "toto\n\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto\r\n"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[8] = "toto\n\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto\r\n"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "toto\n\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto\r\n\r\n"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[10] = "toto\n\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto\r\n\r\n"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\n\ntoto";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\n\r\nto"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "\n\ntoto";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\n\r\ntoto"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\ntoto\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\ntoto"));
//        BOOST_CHECK(result_size == 9);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter2)
//{
//    const bool cancel_if_buffer_too_small = true;
//
//    size_t result_size;
//
//    {
//        char s[6] = "toto\n";
//        BOOST_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        BOOST_CHECK(result_size == 7);
//    }
//
//    {
//        char s[7] = "toto\n\n";
//        BOOST_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[8] = "toto\n\n";
//        BOOST_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "toto\n\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto\r\n\r\n"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[10] = "toto\n\n";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "toto\r\n\r\n"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\n\ntoto";
//        BOOST_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[9] = "\n\ntoto";
//        BOOST_CHECK((in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) == s) && !strcmp(s, "\r\n\r\ntoto"));
//        BOOST_CHECK(result_size == 9);
//    }
//
//    {
//        char s[7] = "\ntoto\n";
//        BOOST_CHECK(in_place_linux_to_windows_newline_convert(s, sizeof(s), cancel_if_buffer_too_small, result_size) != s);
//        BOOST_CHECK(result_size == 9);
//    }
//}

BOOST_AUTO_TEST_CASE(TestLinuxToWindowsNewLineConverter)
{
/*
    BOOST_CHECK(linux_to_windows_newline_convert(nullptr, 0, nullptr, 0) == 0);

    {
        char d[8];
        BOOST_CHECK(linux_to_windows_newline_convert(nullptr, 0, d, sizeof(d)) == 0);
    }
*/

    {
        char d[8];
        BOOST_CHECK(linux_to_windows_newline_convert("text", 0, d, sizeof(d)) == 0);
    }

    {
        CHECK_EXCEPTION_ERROR_ID(
            linux_to_windows_newline_convert("text", 4, nullptr, 0),
            ERR_STREAM_MEMORY_TOO_SMALL
        );
    }

    {
        char d[2];
        CHECK_EXCEPTION_ERROR_ID(
            linux_to_windows_newline_convert("text", 4, d, sizeof(d)),
            ERR_STREAM_MEMORY_TOO_SMALL
        );
    }

    {
        char d[8];
        BOOST_CHECK(linux_to_windows_newline_convert("", 1, d, sizeof(d)) == 1);
        BOOST_CHECK(!memcmp(d, "\0", 1));
    }

    {
        char d[8];
        BOOST_CHECK(linux_to_windows_newline_convert("toto", 4, d, sizeof(d)) == 4);
        BOOST_CHECK(!memcmp(d, "toto", 4));
    }

    {
        char d[8];
        BOOST_CHECK_EQUAL(linux_to_windows_newline_convert("toto\n", 5, d, sizeof(d)), 6);
        BOOST_CHECK(!memcmp(d, "toto\r\n", 6));
    }

    {
        char d[8];
        BOOST_CHECK_EQUAL(linux_to_windows_newline_convert("toto\n", 6, d, sizeof(d)), 7);
        BOOST_CHECK(!memcmp(d, "toto\r\n", 7));
    }

    {
        char d[8];
        BOOST_CHECK(linux_to_windows_newline_convert("\ntoto", 5, d, sizeof(d)) == 6);
        BOOST_CHECK(!memcmp(d, "\r\ntoto", 6));
    }

    {
        char d[8];
        BOOST_CHECK(linux_to_windows_newline_convert("to\nto", 5, d, sizeof(d)) == 6);
        BOOST_CHECK(!memcmp(d, "to\r\nto", 6));
    }

    {
        char d[12];
        BOOST_CHECK(linux_to_windows_newline_convert("\nto\nto\n", 7, d, sizeof(d)) == 10);
        BOOST_CHECK(!memcmp(d, "\r\nto\r\nto\r\n", 10));
    }
}

inline bool ends_with(char const * s1, char const * s2) {
    return ends_with(s1, strlen(s1), s2, strlen(s2));
}

inline bool ends_case_with(char const * s1, char const * s2) {
    return ends_case_with(s1, strlen(s1), s2, strlen(s2));
}

BOOST_AUTO_TEST_CASE(TestEndsWith)
{
    BOOST_CHECK(!ends_with("", "ini"));
    BOOST_CHECK(!ends_with("ni", "ini"));
    BOOST_CHECK(!ends_case_with("init", "ini"));
    BOOST_CHECK(!ends_with("rdpproxy.conf", "ini"));
    BOOST_CHECK(!ends_with("Ini", "ini"));
    BOOST_CHECK(!ends_with("RDPPROXY.INI", "ini"));

    BOOST_CHECK(ends_with("ini", "ini"));
    BOOST_CHECK(ends_with(".ini", "ini"));
    BOOST_CHECK(ends_with("rdpproxy.ini", "ini"));

    BOOST_CHECK(ends_with("RDPPROXY.INI", ""));
    BOOST_CHECK(ends_with("", ""));
}

BOOST_AUTO_TEST_CASE(TestEndsCaseWith)
{
    BOOST_CHECK(!ends_case_with("", "ini"));
    BOOST_CHECK(!ends_case_with("ni", "ini"));
    BOOST_CHECK(!ends_case_with("NI", "ini"));
    BOOST_CHECK(!ends_case_with("init", "ini"));
    BOOST_CHECK(!ends_case_with("INIT", "ini"));
    BOOST_CHECK(!ends_case_with("rdpproxy.conf", "ini"));
    BOOST_CHECK(!ends_case_with("RDPPROXY.CONF", "ini"));

    BOOST_CHECK(ends_case_with("ini", "ini"));
    BOOST_CHECK(ends_case_with("Ini", "ini"));
    BOOST_CHECK(ends_case_with(".ini", "ini"));
    BOOST_CHECK(ends_case_with(".INI", "ini"));
    BOOST_CHECK(ends_case_with("rdpproxy.ini", "ini"));
    BOOST_CHECK(ends_case_with("RDPPROXY.INI", "ini"));

    BOOST_CHECK(ends_case_with("RDPPROXY.INI", ""));
    BOOST_CHECK(ends_case_with("", ""));
}

BOOST_AUTO_TEST_CASE(TestSOHSeparatedStringsToMultiSZ)
{
    char dest[16];

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "");
    BOOST_CHECK(!memcmp(dest, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234");
    BOOST_CHECK(!memcmp(dest, "1234\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "12345678901234567890");
    BOOST_CHECK(!memcmp(dest, "12345678901234\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234\x01OPQR");
    BOOST_CHECK(!memcmp(dest, "1234\x00OPQR\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "12345678\x01OPQRSTUV");
    BOOST_CHECK(!memcmp(dest, "12345678\x00OPQRS\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234\x01\x01");
    BOOST_CHECK(!memcmp(dest, "1234\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234\x01\x01OPQR");
    BOOST_CHECK(!memcmp(dest, "1234\x00\x00OPQR\x00\x00\x00\x00\x00\x00", sizeof(dest)));
}

BOOST_AUTO_TEST_CASE(TestMultiSZCopy)
{
    char dest[16];

    MultiSZCopy(dest, sizeof(dest), "\x00");
    BOOST_CHECK(!memcmp(dest, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "1234\x00");
    BOOST_CHECK(!memcmp(dest, "1234\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "12345678901234567890\x00");
    BOOST_CHECK(!memcmp(dest, "12345678901234\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "1234\x00OPQR\x00");
    BOOST_CHECK(!memcmp(dest, "1234\x00OPQR\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "12345678\x00OPQRSTUV\x00");
    BOOST_CHECK(!memcmp(dest, "12345678\x00OPQRS\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "12345678\x00\x00OPQRSTUV\x00");
    BOOST_CHECK(!memcmp(dest, "12345678\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));
}
