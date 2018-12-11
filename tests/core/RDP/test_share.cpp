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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestRDP
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"
#include "core/RDP/share.hpp"

RED_AUTO_TEST_CASE(TestSendShareControlAndData)
{
    StaticOutStream<65536> stream;
    memset(stream.get_data(), 0, 65536);

    ShareData sdata(stream);
    sdata.emit_begin(PDUTYPE2_UPDATE, 0x12345678, RDP::STREAM_MED);
    sdata.emit_end();

    StaticOutStream<256> sctrl_header;
    ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, 1, stream.get_offset());
    RED_CHECK_EQUAL(6, sctrl_header.get_offset());

    uint8_t * data = sctrl_header.get_data();
    RED_CHECK_EQUAL(0x12, data[0] + data[1]*256);
    RED_CHECK_EQUAL((0x10 | PDUTYPE_DATAPDU), data[2] + data[3]*256);
    RED_CHECK_EQUAL(1, data[4] + data[5]*256);

    data = stream.get_data();
    RED_CHECK_EQUAL(0x12345678,
            ((data[3]*256 + data[2])*256 + data[1])*256 + data[0]);
    RED_CHECK_EQUAL(0, data[4]);
    RED_CHECK_EQUAL(2, data[5]);
    RED_CHECK_EQUAL(18, data[6] + data[7]*256);
    RED_CHECK_EQUAL(static_cast<uint8_t>(PDUTYPE2_UPDATE), data[8]);
    RED_CHECK_EQUAL(0, data[9]);
    RED_CHECK_EQUAL(0, (data[11] << 8) + data[10]);


    // concatenate Data and control before checking read
    StaticOutStream<65536> stream2;
    stream2.out_copy_bytes(sctrl_header.get_bytes());
    stream2.out_copy_bytes(stream.get_bytes());

    InStream in_stream2(stream2.get_bytes());

    ShareControl_Recv sctrl2(in_stream2);
    RED_CHECK_EQUAL(unsigned(PDUTYPE_DATAPDU), unsigned(sctrl2.pduType));
    RED_CHECK_EQUAL(18, sctrl2.totalLength);
    RED_CHECK_EQUAL(1, sctrl2.PDUSource);
}


RED_AUTO_TEST_CASE(TestX224SendShareControlAndData)
{
    StaticOutStream<65536> stream;
    ShareData sdata(stream);
    sdata.emit_begin(PDUTYPE2_UPDATE, 0x12345678, RDP::STREAM_MED);
    sdata.emit_end();

    StaticOutStream<256> sctrl_header;
    ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, 1, stream.get_offset());
    RED_CHECK_EQUAL(6, sctrl_header.get_offset());

    uint8_t * data = sctrl_header.get_data();
    RED_CHECK_EQUAL(0x12, data[0] + data[1]*256);
    RED_CHECK_EQUAL((0x10 | PDUTYPE_DATAPDU), data[2] + data[3]*256);
    RED_CHECK_EQUAL(1, data[4] + data[5]*256);

    data = stream.get_data();
    RED_CHECK_EQUAL(0x12345678,
            ((data[3]*256 + data[2])*256 + data[1])*256 + data[0]);
    RED_CHECK_EQUAL(0, data[4]);
    RED_CHECK_EQUAL(2, data[5]);
    RED_CHECK_EQUAL(18, data[6] + data[7]*256);
    RED_CHECK_EQUAL(static_cast<uint32_t>(PDUTYPE2_UPDATE), data[8]);
    RED_CHECK_EQUAL(0, data[9]);
    RED_CHECK_EQUAL(0, (data[11] << 8) + data[10]);

    // concatenate Data and control before checking read
    StaticOutStream<65536> stream2;
    stream2.out_copy_bytes(sctrl_header.get_bytes());
    stream2.out_copy_bytes(stream.get_bytes());

    InStream in_stream2(stream2.get_bytes());

    ShareControl_Recv sctrl2(in_stream2);
    RED_CHECK_EQUAL(unsigned(PDUTYPE_DATAPDU), unsigned(sctrl2.pduType));
    RED_CHECK_EQUAL(18, sctrl2.totalLength);
    RED_CHECK_EQUAL(1, sctrl2.PDUSource);
}
