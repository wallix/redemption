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
#define BOOST_TEST_MODULE TestCredSSP
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "RDP/nla/credssp.hpp"

BOOST_AUTO_TEST_CASE(TestTSRequest)
{
    uint8_t packet[] = {
        0x30, 0x37, 0xa0, 0x03, 0x02, 0x01, 0x02, 0xa1,
        0x30, 0x30, 0x2e, 0x30, 0x2c, 0xa0, 0x2a, 0x04,
        0x28, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50,
        0x00, 0x01, 0x00, 0x00, 0x00, 0xb7, 0x82, 0x08,
        0xe2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x05, 0x01, 0x28, 0x0a, 0x00, 0x00, 0x00,
        0x0f
    };

    BStream s;

    s.out_copy_bytes(packet, sizeof(packet));
    s.mark_end();
    s.rewind();

    TSRequest ts_req(s);

    BOOST_CHECK_EQUAL(ts_req.version, 2);

    BOOST_CHECK_EQUAL(ts_req.negoTokens.size(), 0x28);
    BOOST_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    BOOST_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0);

    BStream to_send;

    BOOST_CHECK_EQUAL(to_send.size(), 0);
    ts_req.emit(to_send);

    BOOST_CHECK_EQUAL(to_send.size(), 0x37 + 2);
    hexdump_c(to_send.get_data(), to_send.size());

}
