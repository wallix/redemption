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

   Unit test for bitmap class, compression performance

*/

#define RED_TEST_MODULE TestSsl
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "system/ssl_calls.hpp"
#include <cstring>

RED_AUTO_TEST_CASE(TestRC4)
{
    // Test that encrypting several successive small buffers yield the same result
    // as encrypting one larger buffer at once.
    // That's what RC4 is supposed to do, but check it anyway.
    uint8_t source[4096];
    for (size_t i = 0; i < sizeof(source) ; i++){
        source[i] = i;
    }
    const uint8_t initkey[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    RC4_KEY rc4;
    RC4_set_key(&rc4, sizeof(initkey), initkey);
    uint8_t target[4096];
    RC4(&rc4, 4096, source, target);

    RC4_set_key(&rc4, sizeof(initkey), initkey);
    uint8_t target1[4096];
    size_t cut = 0;
    int chunk = 517;
    for(; cut < sizeof(source) - chunk ; cut += chunk){
        RC4(&rc4, chunk, source + cut, target1 + cut);
    }
    RC4(&rc4, sizeof(source) - cut, source + cut, target1 + cut);

    RED_CHECK(0 == memcmp(target1, target, sizeof(target)));
}

