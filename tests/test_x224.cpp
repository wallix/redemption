/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TestX224
#include <boost/test/auto_unit_test.hpp>

#include "stream.hpp"
#include "transport.hpp"
#include "x224.hpp"


BOOST_AUTO_TEST_CASE(TestReceiveSimplePacket)
{
    Stream stream;
    GeneratorTransport gt("\x03\x00\x00\x05\xFF", 5);
    Transport * t = &gt;
    X224In tpdu(t, stream);
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(5, tpdu.tpkt.len);
    BOOST_CHECK_EQUAL(stream.end, stream.data+tpdu.tpkt.len);
    BOOST_CHECK_EQUAL(stream.p, stream.data+X224Packet::TPKT_HEADER_LEN);
}

BOOST_AUTO_TEST_CASE(TestSendSimplePacket)
{
    Stream stream;
    GeneratorTransport gt("\x03\x00\x00\x05\xFF", 5);
    Transport * t = &gt;
    X224Out tpdu(X224Packet::CR_TPDU, stream);
    //------------
    tpdu.end();
    tpdu.send(t);
}
