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
   Author(s): Christophe Grosjean, Raphael Zhou

   Unit test to fast-Path Input Event coder/decoder
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFastPath

#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "testtransport.hpp"
#include "RDP/sec.hpp"
#include "RDP/fastpath.hpp"

BOOST_AUTO_TEST_CASE(TestReceive_FastPathClientInputPDU) {
    uint8_t client_random[] =
/* 0000 */ "\x91\x96\x7b\x5d\xbb\x0f\xe1\xcc\x15\x77\x07\x64\x4b\x3d\x22\xe4" // ..{].....w.dK=". |
/* 0010 */ "\x57\x94\x35\xca\xcd\x6a\xb6\xa3\xe8\x0a\xb9\xe7\x9e\x40\x38\x6d" // W.5..j.......@8m |
        ;
    uint8_t server_random[] =
/* 0000 */ "\x90\x4c\x41\x7f\xb6\xde\x26\x45\xb6\x3a\x5c\x91\x7f\xf8\x88\x42" // .LA...&E.:.....B |
/* 0010 */ "\xb8\x03\x3e\xa8\xef\x6a\x64\xc5\xb5\xab\x06\x27\x5d\x90\x3a\x55" // ..>..jd....'].:U |
        ;
    uint8_t key_block[] =
/* 0000 */ "\x9a\x1c\xb9\x88\xdd\xb6\x69\x2d\x34\xdc\x71\x16\x23\x02\x12\x03" // ......i-4.q.#... |
        ;

    CryptContext decrypt;
    memset(decrypt.key, 0, 16);
    memset(decrypt.update_key, 0, 16);
    decrypt.encryptionMethod = 1;

    memset(key_block, 0, sizeof(key_block));

    decrypt.generate_key(key_block, client_random, server_random, 1);

    const char *payload =
/* 0000 */ "\x94\x18\xda\x95\xbb\x3b\x8f\x7a\x83\x56\xdb\x0f\xc6\xd7\xf1\xe9" // .....;.z.V...... |
/* 0010 */ "\xb3\x49\xd3\x15\x27\x67\x08\x53"                                 // .I..'g.S         |
        ;
    size_t payload_length = 24;

    GeneratorTransport in_t(payload, payload_length);

    BStream in_s(65536);

    FastPath::ClientInputEventPDU_Recv in_cie(in_t, in_s, decrypt);

    BOOST_CHECK_EQUAL(5, in_cie.numEvents);

/*
    uint16_t messageTypes[5] = {
          FastPath::FASTPATH_INPUT_EVENT_SCANCODE
        , FastPath::FASTPATH_INPUT_EVENT_MOUSE
        , FastPath::FASTPATH_INPUT_EVENT_SCANCODE
        , FastPath::FASTPATH_INPUT_EVENT_MOUSE
        , FastPath::FASTPATH_INPUT_EVENT_SCANCODE
    };

    uint8_t byte;
    uint8_t eventCode;

    for (uint8_t i = 0; i < in_cie.numEvents; i++){
        byte = in_cie.payload.in_uint8();

        eventCode = (byte & 0xE0) >> 5;

        BOOST_CHECK_EQUAL(eventCode, messageTypes[i]);

        switch (eventCode){
            case FastPath::FASTPATH_INPUT_EVENT_SCANCODE:
            {
                FastPath::KeyboardEvent_Recv ke(in_cie.payload, byte);
            }
            break;

            case FastPath::FASTPATH_INPUT_EVENT_MOUSE:
            {
                FastPath::MouseEvent_Recv me(in_cie.payload, byte);
            }
            break;

            case FastPath::FASTPATH_INPUT_EVENT_SYNC:
            {
                FastPath::SynchronizeEvent_Recv se(in_cie.payload, byte);
            }
            break;

            default:
                throw Error(ERR_RDP_FASTPATH);
            break;
        }
    }

    BOOST_CHECK_EQUAL(0, in_cie.payload.in_remain());
*/
}

