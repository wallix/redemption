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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAuthentifierNew
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "RDP/nla/asn1/ber.hpp"

BOOST_AUTO_TEST_CASE(TestBERInteger)
{
    BStream s(2048);
    int res;
    uint32_t value;

    res = BER::write_integer(s, 114178754);
    BOOST_CHECK_EQUAL(res, 6);

    s.mark_end();
    s.rewind();

    res = BER::read_integer(s, value);
    BOOST_CHECK_EQUAL(res, true);

    BOOST_CHECK_EQUAL(value, 114178754);

    s.reset();

    res = BER::write_integer(s, 1);
    BOOST_CHECK_EQUAL(res, 3);

    s.mark_end();
    s.rewind();

    res = BER::read_integer(s, value);
    BOOST_CHECK_EQUAL(res, true);

    BOOST_CHECK_EQUAL(value, 1);

    s.reset();

    res = BER::write_integer(s, 52165);
    BOOST_CHECK_EQUAL(res, 5);

    s.mark_end();
    s.rewind();

    res = BER::read_integer(s, value);
    BOOST_CHECK_EQUAL(res, true);

    BOOST_CHECK_EQUAL(value, 52165);

    s.reset();

    res = BER::write_integer(s, 0x0FFF);
    BOOST_CHECK_EQUAL(res, 4);

    s.mark_end();
    s.rewind();

    res = BER::read_integer(s, value);
    BOOST_CHECK_EQUAL(res, true);

    BOOST_CHECK_EQUAL(value, 0x0FFF);

    s.reset();

}

BOOST_AUTO_TEST_CASE(TestBERBool)
{
    BStream s(2048);
    int res;
    bool value;

    BER::write_bool(s, true);
    s.mark_end();
    s.rewind();
    res = BER::read_bool(s, value);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(value, true);

    s.reset();

    BER::write_bool(s, false);
    s.mark_end();
    s.rewind();
    res = BER::read_bool(s, value);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(value, false);

    s.reset();

}

BOOST_AUTO_TEST_CASE(TestBEROctetString)
{
    BStream s(2048);
    int res;
    int value;
    const uint8_t oct_str[] =
        { 1, 2, 3, 4, 5, 6 , 7 };

    BER::write_octet_string(s, oct_str, 7);
    s.mark_end();
    s.rewind();
    res = BER::read_octet_string_tag(s, value);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(value, 7);

    s.reset();

}
