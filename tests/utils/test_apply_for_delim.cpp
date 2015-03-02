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
*   Author(s): Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBase64
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL

#include "apply_for_delim.hpp"
#include "genrandom.hpp"
#include "parser.hpp"
#include <vector>

BOOST_AUTO_TEST_CASE(TestApplyForDelim)
{
    using vector_type = std::vector<std::size_t>;
    const char * s = "a, b, c,d, e , ,  fgh,  ij , ";

    vector_type positions;
    apply_for_delim(s, ',', [&](const char * cs) {
        positions.push_back(cs - s);
    });

    BOOST_CHECK(positions == vector_type({0, 3, 6, 8, 11, 15, 18, 24, 29}));

    positions.clear();
    apply_for_delim(s, ',', [&](const char * cs) {
        positions.push_back(cs - s);
    }, [](char) { return false; });

    BOOST_CHECK(positions == vector_type({0, 2, 5, 8, 10, 14, 16, 22, 28}));

    positions.clear();
    apply_for_delim(s, ',', [&](const char * cs) {
        positions.push_back(cs - s);
    }, [](char c) { return c == ' ' || c == ','; });

    BOOST_CHECK(positions == vector_type({0, 3, 6, 8, 11, 18, 24, 29}));
}

BOOST_AUTO_TEST_CASE(TestApplyForDelimToLong)
{
    using vector_long = std::vector<long>;
    vector_long numbers;
    apply_for_delim("\t16, 2  , 0 , 1,-239 ", ',', [&](const char * cs) {
        numbers.push_back(long_from_cstr(cs));
    });

    BOOST_CHECK(numbers == vector_long({16, 2, 0, 1, -239}));
}
