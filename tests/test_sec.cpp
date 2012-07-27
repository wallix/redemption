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

   Unit test to Mcs PDU coder/decoder
   Using lib boost functions for testing
*/
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSec
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "RDP/sec.hpp"

BOOST_AUTO_TEST_CASE(TestSend_SecExchangePacket)
{
    BStream stream(1024);
    size_t length = 5;

    const char * sec_pkt =
/* 0000 */ "\x01\x00\x00\x00" // 0x00000001 = SEC_EXCHANGE_PKT
           "\x48\x00\x00\x00\xca\xe7\xdf\x85\x01\x42\x02\x47" //....H........B.G
/* 0010 */ "\x28\xfc\x11\x97\x85\xa3\xf9\x40\x73\x97\x33\x2d\x9e\xe4\x0c\x8e" //(......@s.3-....
/* 0020 */ "\xe0\x97\xfc\x17\x24\x4e\x35\x33\xe0\x4e\x7d\xdc\x12\x1d\x41\xf1" //....$N53.N}...A.
/* 0030 */ "\xd8\x17\x86\x0e\x79\x9b\x4f\x44\xb2\x82\xf0\x93\x17\xf8\x59\xc9" //....y.OD......Y.
/* 0040 */ "\x7b\xba\x2a\x22\x59\x45\xa7\x3a\x00\x00\x00\x00\x00\x00\x00\x00" //{.*"YE.:........
    ;

    SEC::SecExchangePacket_Send mcs(stream);
//    BOOST_CHECK_EQUAL(length, stream.size());

    const char * expected = 
        "\x04\x00\x00\x00"  // flags * 4
    ;

//    BOOST_CHECK_EQUAL(0, memcmp(expected, stream.data, length));
}

BOOST_AUTO_TEST_CASE(TestReceive_SecExchangePacket)
{
    BStream stream(1024);
    size_t length = 5;
    GeneratorTransport t(
        "\x04"  // ErectDomainRequest * 4
        "\x01"  // subHeight len
        "\x00"  // subHeight
        "\x01"  // subInterval len
        "\x00"  // subInterval
   , length);
    t.recv(&stream.end, length);

//    SEC::SecExchangePacket_Recv mcs(stream, length);
//    BOOST_CHECK_EQUAL((uint32_t)0, sec.flags);
}
