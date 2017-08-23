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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/SMB2/MessageSyntax.hpp"



RED_AUTO_TEST_CASE(ChangeNotifyResponseEmit)
{
    const size_t len = 8;
    const char data[] =
            "\x01\x00\x02\x00\x03\x00\x00\x00";

    StaticOutStream<128> stream;
    smb2::ChangeNotifyResponse pdu(1, 2, 3);
    pdu.emit(stream);

    std::string const out_data(data, len);
    std::string const expected(reinterpret_cast<const char *>(stream.get_data()), len);
    RED_CHECK_EQUAL(expected, out_data);
}

RED_AUTO_TEST_CASE(ChangeNotifyResponseReceive)
{
    const size_t len = 8;
    const char data[] =
            "\x01\x00\x02\x00\x03\x00\x00\x00";

    InStream in_stream(data, len);
    smb2::ChangeNotifyResponse pdu;
    pdu.receive(in_stream);

    RED_CHECK_EQUAL(pdu.StructureSize, 1);
    RED_CHECK_EQUAL(pdu.OutputBufferOffset, 2);
    RED_CHECK_EQUAL(pdu.OutputBufferLength, 3);
}
