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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define RED_TEST_MODULE TestRefreshRectPDU
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/RefreshRectPDU.hpp"
#include "test_only/transport/test_transport.hpp"

RED_AUTO_TEST_CASE(TestRefreshRectPDU)
{
    const char *payload =
/* 0000 */ "\x03\x00\x00\x2c\x02\xf0\x80\x64\x00\x07\x03\xeb\x70\x1e\x1e\x00" //...,...d....p...
/* 0010 */ "\x17\x00\xf0\x03\xea\x03\x02\x00\x00\x02\x1e\x00\x21\x00\x00\x00" //............!...
/* 0020 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x1f\x03\x57\x02"                 //..........W.
        ;
    size_t payload_length = 44;

    CheckTransport out_t(payload, payload_length);
    CryptContext   encrypt;

    RDP::RefreshRectPDU rrpdu(132074, 7, 0, encrypt);

    rrpdu.addInclusiveRect(0, 0, 800 - 1, 600 - 1);

    rrpdu.emit(out_t);
}
