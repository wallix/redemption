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
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "strutils.hpp"

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
        char s[] = "\r\n\r\nto\r\n\r\nto\r\n\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\n\nto\n\nto\n\n"));
    }
}

BOOST_AUTO_TEST_CASE(TestInPlaceWindowsToLinuxNewLineConverter1)
{
    {
        char s[] = "";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), ""));
    }

    {
        char s[] = "\r\r";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\r\r"));
    }

    {
        char s[] = "\n\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\n\n"));
    }

    {
        char s[] = "\r \n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\r \n"));
    }

    {
        char s[] = "\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\n"));
    }

    {
        char s[] = "\r\n\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\n\n"));
    }

    {
        char s[] = "\r\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\r\n"));
    }

    {
        char s[] = "\n\r\n";
        BOOST_CHECK(!strcmp(in_place_windows_to_linux_newline_convert(s), "\n\n"));
    }
}

BOOST_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter0)
{
    {
        char s[5] = "toto";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "toto"));
    }

    {
        char s[7] = "toto\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "toto\r\n"));
    }

    {
        char s[7] = "\ntoto";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "\r\ntoto"));
    }

    {
        char s[7] = "to\nto";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "to\r\nto"));
    }

    {
        char s[9] = "\nto\nto";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "\r\nto\r\nto"));
    }

    {
        char s[11] = "\nto\nto\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "\r\nto\r\nto\r\n"));
    }

    {
        char s[17] = "\n\nto\n\nto\n\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "\r\n\r\nto\r\n\r\nto\r\n\r\n"));
    }
}

BOOST_AUTO_TEST_CASE(TestInPlaceLinuxToWindowsNewLineConverter1)
{
    {
        char s[6] = "toto\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "toto"));
    }

    {
        char s[7] = "toto\n\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "toto\r\n"));
    }

    {
        char s[8] = "toto\n\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "toto\r\n"));
    }

    {
        char s[9] = "toto\n\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "toto\r\n\r\n"));
    }

    {
        char s[10] = "toto\n\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "toto\r\n\r\n"));
    }

    {
        char s[7] = "\n\ntoto";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "\r\n\r\nto"));
    }

    {
        char s[9] = "\n\ntoto";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "\r\n\r\ntoto"));
    }

    {
        char s[7] = "\ntoto\n";
        BOOST_CHECK(!strcmp(in_place_linux_to_windows_newline_convert(s, sizeof(s)), "\r\ntoto"));
    }
}