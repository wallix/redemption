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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE TestArrayView
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "utils/array_view.hpp"

#include <string>

namespace {

int test_ambiguous(array_view_const_char) { return 1; }
int test_ambiguous(array_view_const_s8) { return 2; }
int test_ambiguous(array_view_const_u8) { return 3; }

}

BOOST_AUTO_TEST_CASE(TestArrayView)
{
    char a8[3];
    int8_t as8[3];
    uint8_t au8[3];

    BOOST_CHECK_EQUAL(test_ambiguous(a8), 1);
    BOOST_CHECK_EQUAL(test_ambiguous(as8), 2);
    BOOST_CHECK_EQUAL(test_ambiguous(au8), 3);

    BOOST_CHECK_EQUAL(make_array_view(a8).size(), 3);

    std::string s;
    BOOST_CHECK_EQUAL(test_ambiguous(s), 1);

    s = "abc";
    auto av = make_array_view(s);
    BOOST_CHECK_EQUAL(av.size(), s.size());
    BOOST_CHECK_EQUAL(av.data(), s.data());
    BOOST_CHECK_EQUAL(av[0], s[0]);
    BOOST_CHECK_EQUAL(av[1], s[1]);
    BOOST_CHECK_EQUAL(av[2], s[2]);
    BOOST_CHECK_EQUAL(av.end() - av.begin(), 3);

    {
        auto const av_p = make_array_view(&s[0], &s[3]);
        BOOST_CHECK_EQUAL(static_cast<void const *>(av_p.data()), static_cast<void const *>(av.data()));
        BOOST_CHECK_EQUAL(av_p.size(), av.size());
        BOOST_CHECK_EQUAL(av_p[0], av[0]);
    }

    BOOST_CHECK_EQUAL(make_array_view("abc").size(), 4);
    BOOST_CHECK_EQUAL(cstr_array_view("abc").size(), 3);
    BOOST_CHECK_EQUAL(make_array_view(av.data(), 1).size(), 1);

    BOOST_CHECK(array_view_char{nullptr}.empty());

}
