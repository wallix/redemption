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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestXXX
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/nego.hpp"
#include "testtransport.hpp"

BOOST_AUTO_TEST_CASE(TestNego)
{
    const char client[65000] =
/* 0000 */ "\x03\x00\x00\x2a\x25\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...*%......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x74\x65\x73\x74" //e: mstshash=test
/* 0020 */ "\x0d\x0a\x01\x00\x08\x00\x03\x00\x00\x00"                         //..........
        ;
    const char server[65000] = {};
    TestTransport logtrans("test", server, sizeof(server), client, sizeof(client));
    RdpNego nego(true, &logtrans, "test", true);
    nego.server_event(true);
}


BOOST_AUTO_TEST_CASE(TestNego2)
{
    const char client[65000] =
/* 0000 */ "\x03\x00\x00\x2a\x25\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...*%......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x74\x65\x73\x74" //e: mstshash=test
/* 0020 */ "\x0d\x0a\x01\x00\x08\x00\x03\x00\x00\x00"                         //..........
        ;
    const char server[65000] = {};
    TestTransport logtrans("test", client, sizeof(client), server, sizeof(server));
    RdpNego nego(true, &logtrans, "test", true);
}
