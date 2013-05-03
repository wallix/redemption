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

#define LOGPRINT
#include "log.hpp"

#include "stream.hpp"
#include "RDP/x224.hpp"
#include "RDP/rdp.hpp"


BOOST_AUTO_TEST_CASE(TestSendShareControlAndData)
{
    BStream stream(65536);
    memset(stream.data, 0, 65536);

    ShareData sdata(stream);
    sdata.emit_begin(PDUTYPE2_UPDATE, 0x12345678, RDP::STREAM_MED);
    sdata.emit_end();

    BStream sctrl_header(256);
    ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, 1, stream.size());
    BOOST_CHECK_EQUAL(6, sctrl_header.size());
    
    uint8_t * data = sctrl_header.data;
    BOOST_CHECK_EQUAL(0x12, data[0] + data[1]*256);
    BOOST_CHECK_EQUAL(0x10 | PDUTYPE_DATAPDU, data[2] + data[3]*256);
    BOOST_CHECK_EQUAL(1, data[4] + data[5]*256);

    data = stream.data;
    BOOST_CHECK_EQUAL(0x12345678,
            ((data[3]*256 + data[2])*256 + data[1])*256 + data[0]);
    BOOST_CHECK_EQUAL(0, data[4]);
    BOOST_CHECK_EQUAL(2, data[5]);
    BOOST_CHECK_EQUAL(4, data[6] + data[7]*256);
    BOOST_CHECK_EQUAL((uint8_t)PDUTYPE2_UPDATE, data[8]);
    BOOST_CHECK_EQUAL(0, data[9]);
    BOOST_CHECK_EQUAL(0, (data[11] << 8) + data[10]);
    

    // concatenate Data and control before checking read
    BStream stream2(65536);
    stream2.out_copy_bytes(sctrl_header.data, sctrl_header.size());
    stream2.out_copy_bytes(stream.data, stream.size());
    stream2.mark_end();
    stream2.p = stream2.data;

    ShareControl_Recv sctrl2(stream2);
    BOOST_CHECK_EQUAL((unsigned)PDUTYPE_DATAPDU, (unsigned)sctrl2.pdu_type1);
    BOOST_CHECK_EQUAL(18, sctrl2.len);
    BOOST_CHECK_EQUAL(1, sctrl2.mcs_channel);
}


BOOST_AUTO_TEST_CASE(TestX224SendShareControlAndData)
{
    BStream stream(65536);
    ShareData sdata(stream);
    sdata.emit_begin(PDUTYPE2_UPDATE, 0x12345678, RDP::STREAM_MED);
    sdata.emit_end();

    BStream sctrl_header(256);
    ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, 1, stream.size());
    BOOST_CHECK_EQUAL(6, sctrl_header.size());

    uint8_t * data = sctrl_header.data;
    BOOST_CHECK_EQUAL(0x12, data[0] + data[1]*256);
    BOOST_CHECK_EQUAL(0x10 | PDUTYPE_DATAPDU, data[2] + data[3]*256);
    BOOST_CHECK_EQUAL(1, data[4] + data[5]*256);

    data = stream.data;
    BOOST_CHECK_EQUAL(0x12345678,
            ((data[3]*256 + data[2])*256 + data[1])*256 + data[0]);
    BOOST_CHECK_EQUAL(0, data[4]);
    BOOST_CHECK_EQUAL(2, data[5]);
    BOOST_CHECK_EQUAL(4, data[6] + data[7]*256);
    BOOST_CHECK_EQUAL((uint8_t)PDUTYPE2_UPDATE, data[8]);
    BOOST_CHECK_EQUAL(0, data[9]);
    BOOST_CHECK_EQUAL(0, (data[11] << 8) + data[10]);

    // concatenate Data and control before checking read
    BStream stream2(65536);
    stream2.out_copy_bytes(sctrl_header.data, sctrl_header.size());
    stream2.out_copy_bytes(stream.data, stream.size());
    stream2.mark_end();
    stream2.p = stream2.data;

    ShareControl_Recv sctrl2(stream2);
    BOOST_CHECK_EQUAL((unsigned)PDUTYPE_DATAPDU, (unsigned)sctrl2.pdu_type1);
    BOOST_CHECK_EQUAL(18, sctrl2.len);
    BOOST_CHECK_EQUAL(1, sctrl2.mcs_channel);
}
