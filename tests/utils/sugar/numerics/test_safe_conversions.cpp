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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSplitter
#include "system/redemption_unit_tests.hpp"

#include "utils/sugar/numerics/safe_conversions.hpp"
#include <string>

BOOST_AUTO_TEST_CASE(TestTrim)
{
    BOOST_CHECK_EQUAL(int(saturated_cast<signed char>(1233412)), 127);
    BOOST_CHECK_EQUAL(int(saturated_cast<signed char>(-1233412)), -128);
    BOOST_CHECK_EQUAL(unsigned(saturated_cast<unsigned char>(1233412)), 255);
    BOOST_CHECK_EQUAL(unsigned(saturated_cast<unsigned char>(-1233412)), 0);

    checked_cast<char>(12);

    BOOST_CHECK_EQUAL(int(saturated_int<signed char>(122312)), 127);
    BOOST_CHECK_EQUAL(int(saturated_int<signed char>(122312) = -3213), -128);

    BOOST_CHECK_EQUAL(int(checked_int<signed char>(12)), 12);
    BOOST_CHECK_EQUAL(int(checked_int<signed char>(12) = 13), 13);
}
