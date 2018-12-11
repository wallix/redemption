/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define RED_TEST_MODULE TestGCC
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/SMB2/MessageSyntax.hpp"



RED_AUTO_TEST_CASE(ChangeNotifyResponse)
{
    auto data = "\x01\x00\x02\x00\x03\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        smb2::ChangeNotifyResponse pdu(1, 2, 3);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        smb2::ChangeNotifyResponse pdu;
        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.StructureSize, 1);
        RED_CHECK_EQUAL(pdu.OutputBufferOffset, 2);
        RED_CHECK_EQUAL(pdu.OutputBufferLength, 3);
    }
}
