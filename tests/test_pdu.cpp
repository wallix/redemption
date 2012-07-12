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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   T.124 Generic Conference Control (GCC) Unit Test

   Shamelessly adapted of test_gcc.c from FreedRDP (C) Marc-Andre Moreau

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestPDU
#include <boost/test/auto_unit_test.hpp>

//#define LOGPRINT
#include "log.hpp"

#include "transport.hpp"
#include "RDP/pdu.hpp"

BOOST_AUTO_TEST_CASE(Test_pdu_connection_request)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xE0\x00\x00\x00\x00\x00", 11);

    RDP::PDU rdp_pdu(stream);
    rdp_pdu.recv_begin(&t);
    rdp_pdu.recv_end();

    BOOST_CHECK_EQUAL((uint8_t)RDP::PDU::CONNECTION_REQUEST, (uint8_t)rdp_pdu.pduType); 
}

BOOST_AUTO_TEST_CASE(Test_pdu_connection_confirm)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xD0\x00\x00\x00\x00\x00", 11);

    RDP::PDU rdp_pdu(stream);
    rdp_pdu.recv_begin(&t);
    rdp_pdu.recv_end();

    BOOST_CHECK_EQUAL((uint8_t)RDP::PDU::CONNECTION_CONFIRM, (uint8_t)rdp_pdu.pduType); 
}

BOOST_AUTO_TEST_CASE(Test_pdu_disconnect_request)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0B\x06\x80\x00\x00\x00\x00\x01", 11);

    RDP::PDU rdp_pdu(stream);
    rdp_pdu.recv_begin(&t);
    rdp_pdu.recv_end();

    BOOST_CHECK_EQUAL((uint8_t)RDP::PDU::DISCONNECT_REQUEST, (uint8_t)rdp_pdu.pduType); 
}

BOOST_AUTO_TEST_CASE(Test_pdu_error_tpdu)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0D\x08\x70\x00\x00\x02\xC1\x02\x06\x22", 13);

    RDP::PDU rdp_pdu(stream);
    rdp_pdu.recv_begin(&t);
    rdp_pdu.recv_end();

    BOOST_CHECK_EQUAL((uint8_t)RDP::PDU::ERROR_TPDU, (uint8_t)rdp_pdu.pduType); 
}
