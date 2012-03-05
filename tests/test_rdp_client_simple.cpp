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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRdpClientSimple
#include <boost/test/auto_unit_test.hpp>
#include "./test_orders.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "constants.hpp"
#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/sec.hpp"


BOOST_AUTO_TEST_CASE(TestDecodePacket)
{
    Stream stream(65536);
//    GeneratorTransport t(
//        "\x03\x00\x00\x3d"
//        "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x2e\x08\x00\x00\x00\x05"
//        "\xc2\x9f\x8f\x9c\x6a\xc1\xf0\x2e\x14\xba\x9f\x05\xd2\x72\xc7\xe9"
//        "\x63\x15\x71\xd5\x73\xb0\xfe\xe7\xba\x9e\x75\xa0\x68\x67\xa9\x3f"
//        "\x7b\xfa\x9a\x3b\x15\x9f\x89\x32\xd4"
//        , 61);
    const char * name = "RDP Target";
    int sck = connect("10.10.14.78", 3389, name);
    int verbose = 255;
    SocketTransport t(name, sck, verbose);
    wait_obj back_event(t.sck);
    struct client_mod * mod = new mod_rdp(t,
                        back_event,
                        "qa\\administrateur",
                        "S3cur3!1nux",
                        *this->front,
                        "laptop",
                        0x040C);


}
