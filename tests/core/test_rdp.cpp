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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRDP
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "stream.hpp"
#include "RDP/x224.hpp"
#include "RDP/rdp.hpp"


BOOST_AUTO_TEST_CASE(TestSendShareControlAndData)
{
    BStream stream(65536);
    memset(stream.data, 0, 65536);

    ShareControl sctrl(stream);
    sctrl.emit_begin(PDUTYPE_DATAPDU, 1);
    ShareData sdata(stream);
    sdata.emit_begin(PDUTYPE2_UPDATE, 0x12345678, RDP::STREAM_MED);
    sdata.emit_end();
    sctrl.emit_end();

    uint8_t * data = stream.data;
    BOOST_CHECK_EQUAL(0x12, data[0] + data[1]*256);
    BOOST_CHECK_EQUAL(0x10 | PDUTYPE_DATAPDU, data[2] + data[3]*256);
    BOOST_CHECK_EQUAL(1, data[4] + data[5]*256);
    BOOST_CHECK_EQUAL(0x12345678,
            ((data[9]*256 + data[8])*256 + data[7])*256 + data[6]);
    BOOST_CHECK_EQUAL(0, data[10]);
    BOOST_CHECK_EQUAL(2, data[11]);
    BOOST_CHECK_EQUAL(4, data[12] + data[13]*256);
    BOOST_CHECK_EQUAL((uint8_t)PDUTYPE2_UPDATE, data[14]);
    BOOST_CHECK_EQUAL(0, data[15]);
    BOOST_CHECK_EQUAL(0, (data[17] << 8) + data[16]);
}


BOOST_AUTO_TEST_CASE(TestX224SendShareControlAndData)
{
    BStream stream(65536);
    ShareControl sctrl(stream);
    sctrl.emit_begin(PDUTYPE_DATAPDU, 1);
    ShareData sdata(stream);
    sdata.emit_begin(PDUTYPE2_UPDATE, 0x12345678, RDP::STREAM_MED);
    sdata.emit_end();
    sctrl.emit_end();
    

    uint8_t * data = stream.data;
    BOOST_CHECK_EQUAL(0x12, data[0] + data[1]*256);
    BOOST_CHECK_EQUAL(0x10 | PDUTYPE_DATAPDU, data[2] + data[3]*256);
    BOOST_CHECK_EQUAL(1, data[4] + data[5]*256);
    BOOST_CHECK_EQUAL(0x12345678,
            ((data[9]*256 + data[8])*256 + data[7])*256 + data[6]);
    BOOST_CHECK_EQUAL(0, data[10]);
    BOOST_CHECK_EQUAL(2, data[11]);
    BOOST_CHECK_EQUAL(4, data[12] + data[13]*256);
    BOOST_CHECK_EQUAL((uint8_t)PDUTYPE2_UPDATE, data[14]);
    BOOST_CHECK_EQUAL(0, data[15]);
    BOOST_CHECK_EQUAL(0, (data[17] << 8) + data[16]);
}
