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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Unit test to Slow-Path Input Event coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestSlowPath

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"
#include "test_only/transport/test_transport.hpp"
#include "core/RDP/slowpath.hpp"

RED_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU) {
    const char *payload =
/* 0000 */ "\x08\x00\x00\x00\xae\xcb\x72\x01\x04\x00\x00\x80\x0f\x00\x00\x00" // ......r......... |
/* 0010 */ "\xae\xcb\x72\x01\x00\x00\x00\x00\x02\x00\x00\x00\xae\xcb\x72\x01" // ..r...........r. |
/* 0020 */ "\x04\x00\x00\x80\x0f\x00\x00\x00\xae\xcb\x72\x01\x01\x80\x00\x08" // ..........r..... |
/* 0030 */ "\xe6\x01\x6a\x01\xb8\xcb\x72\x01\x01\x80\x00\x08\xe0\x01\x72\x01" // ..j...r.......r. |
/* 0040 */ "\xb8\xcb\x72\x01\x01\x80\x00\x08\xd9\x01\x7b\x01\xc2\xcb\x72\x01" // ..r.......{...r. |
/* 0050 */ "\x01\x80\x00\x08\xd2\x01\x86\x01\xcc\xcb\x72\x01\x01\x80\x00\x08" // ..........r..... |
/* 0060 */ "\xc7\x01\x92\x01"                                                 // ....             |
        ;
    size_t payload_length = 100;

    GeneratorTransport in_t(payload, payload_length);
    CheckTransport     out_t(payload, payload_length);

    StaticInStream<65536> in_s;
    StaticOutStream<65536> out_s;

    {
        auto end = const_cast<uint8_t*>(in_s.get_data());
        in_t.recv_boom(end, payload_length);
    }

    SlowPath::ClientInputEventPDU_Recv in_cie(in_s);

    RED_CHECK_EQUAL(8, in_cie.numEvents);
    RED_CHECK_EQUAL(96, in_cie.payload.get_capacity());

    SlowPath::ClientInputEventPDU_Send out_cie(out_s, in_cie.numEvents);

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

    for (uint16_t i = 0; i < in_cie.numEvents; i++) {
        SlowPath::InputEvent_Recv in_ie(in_cie.payload);

        RED_CHECK_EQUAL(messageTypes[i], in_ie.messageType);
        RED_CHECK_EQUAL(6,               in_ie.payload.get_capacity());

        SlowPath::InputEvent_Send out_ie(out_s, in_ie.eventTime, in_ie.messageType);

        switch (in_ie.messageType) {
        case SlowPath::INPUT_EVENT_SCANCODE:
        {
            SlowPath::KeyboardEvent_Recv in_ke(in_ie.payload);

            SlowPath::KeyboardEvent_Send out_ke(out_s, in_ke.keyboardFlags, in_ke.keyCode);
        }

        break;

        case SlowPath::INPUT_EVENT_SYNC:
        {
            SlowPath::SynchronizeEvent_Recv in_se(in_ie.payload);

            SlowPath::SynchronizeEvent_Send out_se(out_s, in_se.toggleFlags);
        }
        break;

        case SlowPath::INPUT_EVENT_MOUSE:
        {
            SlowPath::MouseEvent_Recv in_me(in_ie.payload);

            SlowPath::MouseEvent_Send out_me(out_s, in_me.pointerFlags, in_me.xPos, in_me.yPos);
        }
        break;

        default:
            RED_CHECK(false);
        break;
        }
    }

    out_t.send(out_s.get_bytes());
} // RED_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU)


RED_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU2) {
    const char *payload =
/* 0000 */ "\x04\x00\x00\x00\xd6\xcb\x72\x01\x04\x00\x00\x80\x0f\x00\x00\x00" // ......r......... |
/* 0010 */ "\xd6\xcb\x72\x01\x00\x00\x00\x00\x02\x00\x00\x00\xd6\xcb\x72\x01" // ..r...........r. |
/* 0020 */ "\x04\x00\x00\x80\x0f\x00\x00\x00\xd6\xcb\x72\x01\x01\x80\x00\x08" // ..........r..... |
/* 0030 */ "\xbb\x01\xa0\x01"                                                 // ....             |
        ;
    size_t payload_length = 52;
    GeneratorTransport in_t(payload, payload_length);
    CheckTransport out_t(payload, payload_length);

    StaticInStream<65536> in_s;
    StaticOutStream<65536> out_s;

    {
        auto * end = const_cast<uint8_t*>(in_s.get_data());
        in_t.recv_boom(end, payload_length);
    }

    SlowPath::ClientInputEventPDU_Recv in_cie(in_s);

    RED_CHECK_EQUAL(4, in_cie.numEvents);
    RED_CHECK_EQUAL(48, in_cie.payload.get_capacity());

    SlowPath::ClientInputEventPDU_Send out_cie(out_s, in_cie.numEvents);

    uint16_t messageTypes[8] = {
          SlowPath::INPUT_EVENT_SCANCODE
        , SlowPath::INPUT_EVENT_SYNC
        , SlowPath::INPUT_EVENT_SCANCODE
        , SlowPath::INPUT_EVENT_MOUSE
    };

    for (uint16_t i = 0; i < in_cie.numEvents; i++) {
        SlowPath::InputEvent_Recv in_ie(in_cie.payload);

        RED_CHECK_EQUAL(messageTypes[i], in_ie.messageType);
        RED_CHECK_EQUAL(6,               in_ie.payload.get_capacity());

        SlowPath::InputEvent_Send out_ie(out_s, in_ie.eventTime, in_ie.messageType);

        switch (in_ie.messageType) {
        case SlowPath::INPUT_EVENT_SCANCODE:
        {
            SlowPath::KeyboardEvent_Recv in_ke(in_ie.payload);

            SlowPath::KeyboardEvent_Send out_ke(out_s, in_ke.keyboardFlags, in_ke.keyCode);
        }

        break;

        case SlowPath::INPUT_EVENT_SYNC:
        {
            SlowPath::SynchronizeEvent_Recv in_se(in_ie.payload);

            SlowPath::SynchronizeEvent_Send out_se(out_s, in_se.toggleFlags);
        }
        break;

        case SlowPath::INPUT_EVENT_MOUSE:
        {
            SlowPath::MouseEvent_Recv in_me(in_ie.payload);

            SlowPath::MouseEvent_Send out_me(out_s, in_me.pointerFlags, in_me.xPos, in_me.yPos);
        }
        break;

        default:
            RED_CHECK(false);
        break;
        }
    }

    out_t.send(out_s.get_bytes());
} // RED_AUTO_TEST_CASE(TestReceive_SlowPathClientInputPDU2)
