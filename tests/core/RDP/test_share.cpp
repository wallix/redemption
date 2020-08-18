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

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"
#include "core/RDP/share.hpp"

RED_AUTO_TEST_CASE(TestSendShareControlAndData)
{
    StaticOutStream<65536> stream {};

    ShareData sdata(stream);
    sdata.emit_begin(PDUTYPE2_UPDATE, 0x12345678, RDP::STREAM_MED);
    sdata.emit_end();

    RED_CHECK(stream.get_produced_bytes() == "\x78\x56\x34\x12\x00\x02\x12\x00\x02\x00\x00\x00"_av);

    StaticOutStream<256> sctrl_header;
    ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, 1, stream.get_offset());

    RED_CHECK(sctrl_header.get_produced_bytes() == "\x12\x00\x17\x00\x01\x00"_av);

    // concatenate Data and control before checking read
    StaticOutStream<65536> stream2;
    stream2.out_copy_bytes(sctrl_header.get_produced_bytes());
    stream2.out_copy_bytes(stream.get_produced_bytes());

    InStream in_stream2(stream2.get_produced_bytes());

    ShareControl_Recv sctrl2(in_stream2);
    RED_CHECK_EQUAL(unsigned(PDUTYPE_DATAPDU), unsigned(sctrl2.pduType));
    RED_CHECK_EQUAL(18, sctrl2.totalLength);
    RED_CHECK_EQUAL(1, sctrl2.PDUSource);
}
