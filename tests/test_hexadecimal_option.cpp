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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestHexadecimalOption
#include <boost/test/auto_unit_test.hpp>

#include "recorder/wrm_recorder_option.hpp"

BOOST_AUTO_TEST_CASE(TestHexadecimalOption)
{
    HexadecimalOption<20> hop;
    bool b = hop.parse("0506070809000A");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(hop.size, 7);
    BOOST_REQUIRE_EQUAL(hop.data[0], 0x05);
    BOOST_REQUIRE_EQUAL(hop.data[1], 0x06);
    BOOST_REQUIRE_EQUAL(hop.data[2], 0x07);
    BOOST_REQUIRE_EQUAL(hop.data[3], 0x08);
    BOOST_REQUIRE_EQUAL(hop.data[4], 0x09);
    BOOST_REQUIRE_EQUAL(hop.data[5], 0x00);
    BOOST_REQUIRE_EQUAL(hop.data[6], 0x0A);
    BOOST_REQUIRE_EQUAL(hop.data[7], 0);
    BOOST_REQUIRE_EQUAL(hop.data[8], 0);
    BOOST_REQUIRE_EQUAL(hop.data[9], 0);
    BOOST_REQUIRE_EQUAL(hop.data[10], 0);
    BOOST_REQUIRE_EQUAL(hop.data[11], 0);
    BOOST_REQUIRE_EQUAL(hop.data[12], 0);
    BOOST_REQUIRE_EQUAL(hop.data[13], 0);
    BOOST_REQUIRE_EQUAL(hop.data[14], 0);
    BOOST_REQUIRE_EQUAL(hop.data[15], 0);
    BOOST_REQUIRE_EQUAL(hop.data[16], 0);
    BOOST_REQUIRE_EQUAL(hop.data[17], 0);
    BOOST_REQUIRE_EQUAL(hop.data[18], 0);
    BOOST_REQUIRE_EQUAL(hop.data[19], 0);

    b = hop.parse("0506070809000a5");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(hop.size, 8);
    BOOST_REQUIRE_EQUAL(hop.data[0], 0x05);
    BOOST_REQUIRE_EQUAL(hop.data[1], 0x06);
    BOOST_REQUIRE_EQUAL(hop.data[2], 0x07);
    BOOST_REQUIRE_EQUAL(hop.data[3], 0x08);
    BOOST_REQUIRE_EQUAL(hop.data[4], 0x09);
    BOOST_REQUIRE_EQUAL(hop.data[5], 0x00);
    BOOST_REQUIRE_EQUAL(hop.data[6], 0x0A);
    BOOST_REQUIRE_EQUAL(hop.data[7], 0x50);
    BOOST_REQUIRE_EQUAL(hop.data[8], 0);
    BOOST_REQUIRE_EQUAL(hop.data[9], 0);
    BOOST_REQUIRE_EQUAL(hop.data[10], 0);
    BOOST_REQUIRE_EQUAL(hop.data[11], 0);
    BOOST_REQUIRE_EQUAL(hop.data[12], 0);
    BOOST_REQUIRE_EQUAL(hop.data[13], 0);
    BOOST_REQUIRE_EQUAL(hop.data[14], 0);
    BOOST_REQUIRE_EQUAL(hop.data[15], 0);
    BOOST_REQUIRE_EQUAL(hop.data[16], 0);
    BOOST_REQUIRE_EQUAL(hop.data[17], 0);
    BOOST_REQUIRE_EQUAL(hop.data[18], 0);
    BOOST_REQUIRE_EQUAL(hop.data[19], 0);

    BOOST_REQUIRE(!HexadecimalOption<5>().parse("0506070809000A"));
}