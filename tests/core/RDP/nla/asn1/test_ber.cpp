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

#define RED_TEST_MODULE TestBER
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/nla/asn1/ber.hpp"

RED_AUTO_TEST_CASE(TestBERInteger)
{
    StaticOutStream<2048> s;
    int res;
    uint32_t value;

    res = BER::write_integer(s, 114178754);
    RED_CHECK_EQUAL(res, 6);


    InStream in_s(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK_EQUAL(res, true);

    RED_CHECK_EQUAL(value, 114178754);

    s.rewind();

    res = BER::write_integer(s, 1);
    RED_CHECK_EQUAL(res, 3);

    in_s = InStream(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK_EQUAL(res, true);

    RED_CHECK_EQUAL(value, 1);

    s.rewind();

    res = BER::write_integer(s, 52165);
    RED_CHECK_EQUAL(res, 5);

    in_s = InStream(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK_EQUAL(res, true);

    RED_CHECK_EQUAL(value, 52165);

    s.rewind();

    res = BER::write_integer(s, 0x0FFF);
    RED_CHECK_EQUAL(res, 4);

    in_s = InStream(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK_EQUAL(res, true);

    RED_CHECK_EQUAL(value, 0x0FFF);

    s.rewind();
}

//RED_AUTO_TEST_CASE(TestBERBool)
//{
//    BStream s(2048);
//    int res;
//    bool value;
//
//    BER::write_bool(s, true);
//    s.mark_end();
//    s.rewind();
//    res = BER::read_bool(s, value);
//    RED_CHECK_EQUAL(res, true);
//    RED_CHECK_EQUAL(value, true);
//
//    s.rewind();
//
//    BER::write_bool(s, false);
//    s.mark_end();
//    s.rewind();
//    res = BER::read_bool(s, value);
//    RED_CHECK_EQUAL(res, true);
//    RED_CHECK_EQUAL(value, false);
//
//    s.rewind();
//
//}

RED_AUTO_TEST_CASE(TestBEROctetString)
{
    StaticOutStream<2048> s;
    int res;
    int value;
    const uint8_t oct_str[] =
        { 1, 2, 3, 4, 5, 6 , 7 };

    BER::write_octet_string(s, oct_str, 7);
    InStream in_s(s.get_bytes());
    res = BER::read_octet_string_tag(in_s, value);
    RED_CHECK_EQUAL(res, true);
    RED_CHECK_EQUAL(value, 7);

    s.rewind();
}


RED_AUTO_TEST_CASE(TestBERContextual)
{
    StaticOutStream<2048> s;
    int res;
    int value;
    uint8_t tag = 0x06;

    BER::write_contextual_tag(s, tag, 3, true);
    InStream in_s(s.get_bytes());
    res = BER::read_contextual_tag(in_s, tag, value, true);
    RED_CHECK_EQUAL(res, true);
    RED_CHECK_EQUAL(value, 3);

    s.rewind();
}
