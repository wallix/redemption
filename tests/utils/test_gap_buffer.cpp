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
  Copyright (C) Wallix 2014
  Author(s): Christophe Grosjean, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGapBuffer
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "gap_buffer.hpp"


BOOST_AUTO_TEST_CASE(TestGapBuffer)
{
    GapBuffer gbuff;

    uint8_t buff[1024];

    gbuff.put_byte(0x61);
    gbuff.put_byte(0x62);
    gbuff.put_byte(0x63);


    size_t length = gbuff.get_buff(buff, sizeof(buff));

    BOOST_CHECK_EQUAL(length, 3);
    BOOST_CHECK_EQUAL(gbuff.size(), 3);

    // hexdump_c(buff, length);
    BOOST_CHECK_EQUAL(memcmp(buff, "\x61\x62\x63" , 3), 0);

    gbuff.insert_bytes((const uint8_t*)"Bonjour Monde", sizeof("Bonjour Monde"));

    BOOST_CHECK_EQUAL(gbuff.p_pos(), 17);

    gbuff.p_forward();

    BOOST_CHECK_EQUAL(gbuff.p_pos(), 17);

    gbuff.p_backward();

    BOOST_CHECK_EQUAL(gbuff.p_pos(), 16);

    length = gbuff.get_buff(buff, sizeof(buff));

    BOOST_CHECK_EQUAL(length, 17);
    BOOST_CHECK_EQUAL(gbuff.size(), 17);

    // hexdump_c(buff, length);
    BOOST_CHECK_EQUAL(memcmp(buff,
                             "\x61\x62\x63\x42\x6f\x6e\x6a\x6f"
                             "\x75\x72\x20\x4d\x6f\x6e\x64\x65\x00" , 17), 0);

    gbuff.p_backward();
    gbuff.p_backward();
    gbuff.p_backward();
    gbuff.p_backward();

    BOOST_CHECK_EQUAL(gbuff.p_pos(), 12);

    gbuff.put_byte(0x41);
    gbuff.put_byte(0x45);
    gbuff.put_byte(0x50);

    BOOST_CHECK_EQUAL(gbuff.p_pos(), 15);

    length = gbuff.get_buff(buff, sizeof(buff));

    BOOST_CHECK_EQUAL(length, 20);
    BOOST_CHECK_EQUAL(gbuff.size(), 20);

    // hexdump_c(buff, length);

    BOOST_CHECK_EQUAL(memcmp(buff,
                             "\x61\x62\x63\x42\x6f\x6e\x6a\x6f"
                             "\x75\x72\x20\x4d\x41\x45\x50\x6f"
                             "\x6e\x64\x65\x00", 20), 0);


    length = gbuff.get_buff(buff, 5, 2);
    // hexdump_c(buff, length);

    BOOST_CHECK_EQUAL(memcmp(buff,
                             "\x63\x42\x6f\x6e\x6a", 5), 0);

    length = gbuff.get_buff(buff, 3, 17);
    // hexdump_c(buff, length);
    BOOST_CHECK_EQUAL(memcmp(buff,
                             "\x64\x65\x00", 3), 0);


}

