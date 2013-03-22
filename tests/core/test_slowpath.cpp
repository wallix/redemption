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

   Unit test to Slow-Path Input Event coder/decoder
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSlowPath

#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "testtransport.hpp"
#include "RDP/slowpath.hpp"

BOOST_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU) {
    size_t payload_length = 100;
    GeneratorTransport t(
/* 0000 */ "\x08\x00\x00\x00\xae\xcb\x72\x01\x04\x00\x00\x80\x0f\x00\x00\x00" // ......r......... |
/* 0010 */ "\xae\xcb\x72\x01\x00\x00\x00\x00\x02\x00\x00\x00\xae\xcb\x72\x01" // ..r...........r. |
/* 0020 */ "\x04\x00\x00\x80\x0f\x00\x00\x00\xae\xcb\x72\x01\x01\x80\x00\x08" // ..........r..... |
/* 0030 */ "\xe6\x01\x6a\x01\xb8\xcb\x72\x01\x01\x80\x00\x08\xe0\x01\x72\x01" // ..j...r.......r. |
/* 0040 */ "\xb8\xcb\x72\x01\x01\x80\x00\x08\xd9\x01\x7b\x01\xc2\xcb\x72\x01" // ..r.......{...r. |
/* 0050 */ "\x01\x80\x00\x08\xd2\x01\x86\x01\xcc\xcb\x72\x01\x01\x80\x00\x08" // ..........r..... |
/* 0060 */ "\xc7\x01\x92\x01"                                                 // ....             |
    , payload_length);

    BStream payload(65536);
    t.recv(&payload.end, payload_length);

    SlowPath::ClientInputEventPDU_Recv cie(payload);

    BOOST_CHECK_EQUAL(8, cie.numEvents);
    BOOST_CHECK_EQUAL(96, cie.payload.size());

    uint16_t messageTypes[8] = {
          SlowPath::INPUT_EVENT_SCANCODE
        , SlowPath::INPUT_EVENT_SYNC
        , SlowPath::INPUT_EVENT_SCANCODE
        , SlowPath::INPUT_EVENT_MOUSE
        , SlowPath::INPUT_EVENT_MOUSE
        , SlowPath::INPUT_EVENT_MOUSE
        , SlowPath::INPUT_EVENT_MOUSE
        , SlowPath::INPUT_EVENT_MOUSE
    };

    for (uint16_t i = 0; i < cie.numEvents; i++) {
        SlowPath::InputEvent_Recv ie(cie.payload);

        BOOST_CHECK_EQUAL(messageTypes[i], ie.messageType);
//        BOOST_CHECK_EQUAL(6,               ie.payload.size());
    }
} // BOOST_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU)


BOOST_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU2) {
    size_t payload_length = 52;
    GeneratorTransport t(
/* 0000 */ "\x04\x00\x00\x00\xd6\xcb\x72\x01\x04\x00\x00\x80\x0f\x00\x00\x00" // ......r......... |
/* 0010 */ "\xd6\xcb\x72\x01\x00\x00\x00\x00\x02\x00\x00\x00\xd6\xcb\x72\x01" // ..r...........r. |
/* 0020 */ "\x04\x00\x00\x80\x0f\x00\x00\x00\xd6\xcb\x72\x01\x01\x80\x00\x08" // ..........r..... |
/* 0030 */ "\xbb\x01\xa0\x01"                                                 // ....             |
    , payload_length);

    BStream payload(65536);
    t.recv(&payload.end, payload_length);

    SlowPath::ClientInputEventPDU_Recv cie(payload);

    BOOST_CHECK_EQUAL(4, cie.numEvents);
    BOOST_CHECK_EQUAL(48, cie.payload.size());
} // BOOST_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU2)
