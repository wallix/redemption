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
#define BOOST_TEST_MODULE TestCountTransport
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "transport/count_transport.hpp"

BOOST_AUTO_TEST_CASE(TestCountTransport)
{
    // Count transport never receive anything or send anything but updates amount of data sent/received
    CountTransport ct;
    BOOST_CHECK_EQUAL(0, ct.get_total_received());
    BOOST_CHECK_EQUAL(0, ct.get_last_quantum_received());
    BOOST_CHECK_EQUAL(0, ct.get_total_sent());
    BOOST_CHECK_EQUAL(0, ct.get_last_quantum_sent());

    char buffer[128] = {};
    char * p = buffer;
    ct.recv(&p, 0);
    BOOST_CHECK_EQUAL(0, ct.get_total_received());
    BOOST_CHECK_EQUAL(0, ct.get_last_quantum_received());

    ct.recv(&p, 1);
    BOOST_CHECK_EQUAL(1, ct.get_total_received());
    BOOST_CHECK_EQUAL(1, ct.get_last_quantum_received());

    ct.tick();

    ct.recv(&p, 3);
    BOOST_CHECK_EQUAL(4, ct.get_total_received());
    BOOST_CHECK_EQUAL(3, ct.get_last_quantum_received());

    BOOST_CHECK_EQUAL(0, ct.get_total_sent());
    BOOST_CHECK_EQUAL(0, ct.get_last_quantum_sent());

    ct.send(buffer, 3);

    BOOST_CHECK_EQUAL(3, ct.get_total_sent());
    BOOST_CHECK_EQUAL(3, ct.get_last_quantum_sent());

    ct.send(buffer, 10);

    BOOST_CHECK_EQUAL(13, ct.get_total_sent());
    BOOST_CHECK_EQUAL(13, ct.get_last_quantum_sent());

    ct.tick();

    BOOST_CHECK_EQUAL(13, ct.get_total_sent());
    BOOST_CHECK_EQUAL(0, ct.get_last_quantum_sent());
}


