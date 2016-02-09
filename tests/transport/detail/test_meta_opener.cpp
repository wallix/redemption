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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestMetaOpener
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL

#include "transport/detail/meta_opener.hpp"

#include <string>

BOOST_AUTO_TEST_CASE(TestReadFilename)
{
    using detail::sread_filename;

    char filename_buf[PATH_MAX + 1];

#define TEST_READ_FILENAME(filename, quoted_filename)                                  \
    sread_filename(std::begin(filename_buf), std::end(filename_buf), quoted_filename); \
    BOOST_CHECK_EQUAL(filename_buf, filename);

    TEST_READ_FILENAME("abcde.txt", "abcde.txt");

    TEST_READ_FILENAME(R"(\abcde.txt)", R"(\\abcde.txt)");
    TEST_READ_FILENAME(R"(abc\de.txt)", R"(abc\\de.txt)");
    TEST_READ_FILENAME(R"(abcde.txt\)", R"(abcde.txt\\)");
    TEST_READ_FILENAME(R"(abc\\de.txt)", R"(abc\\\\de.txt)");
    TEST_READ_FILENAME(R"(\\\\)", R"(\\\\\\\\)");

    TEST_READ_FILENAME(R"( abcde.txt)", R"(\ abcde.txt)");
    TEST_READ_FILENAME(R"(abc de.txt)", R"(abc\ de.txt)");
    TEST_READ_FILENAME(R"(abcde.txt )", R"(abcde.txt\ )");
    TEST_READ_FILENAME(R"(abc  de.txt)", R"(abc\ \ de.txt)");
    TEST_READ_FILENAME(R"(    )", R"(\ \ \ \ )");
}
