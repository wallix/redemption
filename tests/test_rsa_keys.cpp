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

   Unit test to rsa_keys.ini file
   Using lib boost functions, some tests need to be added

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRSAkeys
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "rsa_keys.hpp"
#include <sstream>


BOOST_AUTO_TEST_CASE(TestRSAkeysFromFile)
{
    // test we can read from a file (and not only from a stream)
    Rsakeys test_rsakeys(FIXTURES_PATH "/rsakeys.ini");
}

BOOST_AUTO_TEST_CASE(TestRSAkeysFromLine)
{
    std::stringstream test_stream(
    "[keys]\n"
    "pub_exp=0x05, 0x84, 0xac, 0xad\n"
    "pub_mod=0x54, 0x53, 0x57, 0xca, 0x79, 0x97, 0x31, 0xab, 0x38, 0x87\n"
    "pub_sig=0x65, 0x63, 0x97, 0xca, 0x19, 0x91, 0x41, 0xab, 0x45, 0x77\n"
    "pri_exp=0x65, 0x63, 0x97, 0xca, 0x19, 0x91, 0x41, 0xab, 0x45, 0x77, 0x53, 0x57, 0xca, 0x79, 0x97, 0x31,  0x53, 0x57, 0xca, 0x79, 0x97, 0x31\n"
    );

    Rsakeys rsakeys(test_stream);
}
