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

   Unit test to fast-Path Input Event coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestFastPath

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"
#include "test_only/transport/test_transport.hpp"
#include "core/RDP/x224.hpp"
#include "core/RDP/fastpath.hpp"

RED_AUTO_TEST_CASE(TestReceive_FastPathClientInputPDU) {
    CryptContext decrypt;

    // TODO We should fix that test (and a few other below) to make it independant from transport

    const char *payload =
/* 0000 */ "\x10\x0e\x01\x0f\x62\x01\x0f\x20\x00\x08\xca\x00\x41\x03"         // ....b.. ....A.   |
        ;
    size_t payload_length = 14;

    GeneratorTransport in_t(payload, payload_length);
    CheckTransport     out_t(payload, payload_length);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fx224(in_t, &end, array_size);

    InStream in_s(array, end - array);
    FastPath::ClientInputEventPDU_Recv in_cie(in_s, decrypt);

    RED_CHECK_EQUAL(4, in_cie.numEvents);

    uint16_t messageTypes[4] = {
          FastPath::FASTPATH_INPUT_EVENT_SCANCODE
        , FastPath::FASTPATH_INPUT_EVENT_SYNC
        , FastPath::FASTPATH_INPUT_EVENT_SCANCODE
        , FastPath::FASTPATH_INPUT_EVENT_MOUSE
    };

    uint8_t byte;
    uint8_t eventCode;
    StaticOutStream<65536> out_payload;

    for (uint8_t i = 0; i < in_cie.numEvents; i++){
        byte = in_cie.payload.in_uint8();

        eventCode = (byte & 0xE0) >> 5;

        RED_CHECK_EQUAL(eventCode, messageTypes[i]);

        switch (eventCode){
            case FastPath::FASTPATH_INPUT_EVENT_SCANCODE:
            {
                FastPath::KeyboardEvent_Recv ke(in_cie.payload, byte);
                FastPath::KeyboardEvent_Send(out_payload, ke.eventFlags, ke.keyCode);
            }
            break;

            case FastPath::FASTPATH_INPUT_EVENT_MOUSE:
            {
                FastPath::MouseEvent_Recv me(in_cie.payload, byte);
                FastPath::MouseEvent_Send(out_payload, me.pointerFlags, me.xPos, me.yPos);
            }
            break;

            case FastPath::FASTPATH_INPUT_EVENT_SYNC:
            {
                FastPath::SynchronizeEvent_Recv se(in_cie.payload, byte);
                FastPath::SynchronizeEvent_Send(out_payload, se.eventFlags);
            }
            break;

            default:
                RED_CHECK(!"ERR FASTPATH");
        }
    }

    RED_CHECK_EQUAL(0, in_cie.payload.in_remain());

    StaticOutStream<65536> out_s;

    FastPath::ClientInputEventPDU_Send out_cie(
        out_s, out_payload.get_data(), out_payload.get_offset(), in_cie.numEvents, decrypt, 0, 0);

    out_t.send(out_s.get_bytes());
    out_t.send(out_payload.get_bytes());
}

RED_AUTO_TEST_CASE(TestReceive_FastPathClientInputPDU2) {
    CryptContext decrypt;

    const char *payload =
/* 0000 */ "\x18\x27\x20\x00\x08\x4a\x01\x92\x02\x20\x00\x08\x50\x01\x96\x02" // .' ..J... ..P... |
/* 0010 */ "\x20\x00\x08\x63\x01\x9d\x02\x20\x00\x08\x80\x01\xa6\x02\x20\x00" //  ..c... ...... . |
/* 0020 */ "\x08\xa5\x01\xad\x02\x00\x39"                                     // ......9          |
        ;
    size_t payload_length = 39;

    GeneratorTransport in_t(payload, payload_length);
    CheckTransport     out_t(payload, payload_length);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fx224(in_t, &end, array_size);
    InStream in_s(array, end - array);
    FastPath::ClientInputEventPDU_Recv in_cie(in_s, decrypt);

    RED_CHECK_EQUAL(6, in_cie.numEvents);

    uint16_t messageTypes[6] = {
          FastPath::FASTPATH_INPUT_EVENT_MOUSE
        , FastPath::FASTPATH_INPUT_EVENT_MOUSE
        , FastPath::FASTPATH_INPUT_EVENT_MOUSE
        , FastPath::FASTPATH_INPUT_EVENT_MOUSE
        , FastPath::FASTPATH_INPUT_EVENT_MOUSE
        , FastPath::FASTPATH_INPUT_EVENT_SCANCODE
    };

    uint8_t byte;
    uint8_t eventCode;
    StaticOutStream<65536> out_payload;

    for (uint8_t i = 0; i < in_cie.numEvents; i++){
        byte = in_cie.payload.in_uint8();

        eventCode = (byte & 0xE0) >> 5;

        RED_CHECK_EQUAL(eventCode, messageTypes[i]);

        switch (eventCode){
            case FastPath::FASTPATH_INPUT_EVENT_SCANCODE:
            {
                FastPath::KeyboardEvent_Recv ke(in_cie.payload, byte);

                FastPath::KeyboardEvent_Send(out_payload, ke.eventFlags, ke.keyCode);
            }
            break;

            case FastPath::FASTPATH_INPUT_EVENT_MOUSE:
            {
                FastPath::MouseEvent_Recv me(in_cie.payload, byte);

                FastPath::MouseEvent_Send(out_payload, me.pointerFlags, me.xPos, me.yPos);
            }
            break;

            case FastPath::FASTPATH_INPUT_EVENT_SYNC:
            {
                FastPath::SynchronizeEvent_Recv se(in_cie.payload, byte);

                FastPath::SynchronizeEvent_Send(out_payload, se.eventFlags);
            }
            break;

            default:
                throw Error(ERR_RDP_FASTPATH);
        }
    }

    RED_CHECK_EQUAL(0, in_cie.payload.in_remain());

    StaticOutStream<65536> out_s;

    FastPath::ClientInputEventPDU_Send out_cie(
        out_s, out_payload.get_data(), out_payload.get_offset(), in_cie.numEvents, decrypt, 0, 0);

    out_t.send(out_s.get_bytes());
    out_t.send(out_payload.get_bytes());
}

RED_AUTO_TEST_CASE(TestReceive_FastPathServerUpdatePDU) {
    CryptContext decrypt;

    const char *payload =
/* 0000 */ "\x00\x2e\x03\x00\x00\x01\x20\x00\x01\x00\x01\x00\x00\x00\x00\x00" // ...... ......... |
/* 0010 */ "\x0f\x00\x00\x00\x10\x00\x01\x00\x10\x00\x01\x04\x0a\x00\x0c\x84" // ................ |
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x06\x00\x00"         // ..............   |
        ;
    size_t payload_length = 46;

    GeneratorTransport in_t(payload, payload_length);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fx224(in_t, &end, array_size);
    InStream in_s(array, end - array);
    FastPath::ServerUpdatePDU_Recv in_su(in_s, decrypt, array);

    uint8_t updateCodes[4] = {
          static_cast<uint8_t>(FastPath::UpdateType::SYNCHRONIZE)
        , static_cast<uint8_t>(FastPath::UpdateType::BITMAP)
        , static_cast<uint8_t>(FastPath::UpdateType::SYNCHRONIZE)
        , static_cast<uint8_t>(FastPath::UpdateType::PTR_DEFAULT)
    };

    uint8_t i = 0;

    while (in_su.payload.in_remain()) {
        FastPath::Update_Recv in_upd(in_su.payload, nullptr);

        RED_CHECK_EQUAL(in_upd.updateCode, updateCodes[i++]);

//        FastPath::Update_Send out_upd(in_su.payload);
    }

    RED_CHECK_EQUAL(i, 4);

    RED_CHECK_EQUAL(0, in_su.payload.in_remain());
}

RED_AUTO_TEST_CASE(TestReceive_FastPathServerUpdatePDU2) {
    CryptContext decrypt;

    const char *payload =
/* 0000 */ "\x00\x81\x4a\x00\x0a\x00\x02\x00\x09\x0a\x0c\x80\x04\x60\x03\x51" // ..J..........`.Q |
/* 0010 */ "\x01\x20\x00\x01\x00\x01\x00\x00\x00\x00\x00\x0f\x00\x00\x00\x10" // . .............. |
/* 0020 */ "\x00\x01\x00\x10\x00\x01\x04\x0a\x00\x0c\x84\x00\x00\x00\x00\x00" // ................ |
/* 0030 */ "\x00\x00\x00\x03\x00\x00\x0b\x11\x01\x01\x00\x00\x00\x0a\x00\x0a" // ................ |
/* 0040 */ "\x00\x20\x00\x20\x00\x80\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00" // . . ............ |
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................ |
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x18\x00" // ................ |
/* 0080 */ "\x00\x00\x1c\x00\x00\x00\x1e\x00\x00\x00\x1f\x00\x00\x00\x1f\x80" // ................ |
/* 0090 */ "\x00\x00\x1f\xc0\x00\x00\x1f\xe0\x00\x00\x1f\xf0\x00\x00\x1f\x80" // ................ |
/* 00a0 */ "\x00\x00\x1d\x80\x00\x00\x19\x80\x00\x00\x10\xc0\x00\x00\x00\xc0" // ................ |
/* 00b0 */ "\x00\x00\x00\x60\x00\x00\x00\x60\x00\x00\x00\x30\x00\x00\x00\x30" // ...`...`...0...0 |
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff" // ................ |
/* 00d0 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // ................ |
/* 00e0 */ "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // ................ |
/* 00f0 */ "\xff\xff\xdf\xff\xff\xff\xcf\xff\xff\xff\xc7\xff\xff\xff\xc3\xff" // ................ |
/* 0100 */ "\xff\xff\xc1\xff\xff\xff\xc0\xff\xff\xff\xc0\x7f\xff\xff\xc0\x3f" // ...............? |
/* 0110 */ "\xff\xff\xc0\x1f\xff\xff\xc0\x0f\xff\xff\xc0\x07\xff\xff\xc0\x03" // ................ |
/* 0120 */ "\xff\xff\xc0\x3f\xff\xff\xc0\x3f\xff\xff\xc6\x1f\xff\xff\xce\x1f" // ...?...?........ |
/* 0130 */ "\xff\xff\xdf\x0f\xff\xff\xff\x0f\xff\xff\xff\x87\xff\xff\xff\x87" // ................ |
/* 0140 */ "\xff\xff\xff\xcf\xff\xff\xff\xff\xff\x7a"                         // .........z       |
        ;
    size_t payload_length = 330;

    GeneratorTransport in_t(payload, payload_length);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fx224(in_t, &end, array_size);
    InStream in_s(array, end - array);
    FastPath::ServerUpdatePDU_Recv in_su(in_s, decrypt, array);

    uint8_t updateCodes[4] = {
          static_cast<uint8_t>(FastPath::UpdateType::ORDERS)
        , static_cast<uint8_t>(FastPath::UpdateType::BITMAP)
        , static_cast<uint8_t>(FastPath::UpdateType::SYNCHRONIZE)
        , static_cast<uint8_t>(FastPath::UpdateType::POINTER)
    };

    uint8_t i = 0;

    while (in_su.payload.in_remain()) {
        FastPath::Update_Recv in_upd(in_su.payload, nullptr);

        RED_CHECK_EQUAL(in_upd.updateCode, updateCodes[i++]);
    }

    RED_CHECK_EQUAL(i, 4);

    RED_CHECK_EQUAL(0, in_su.payload.in_remain());
}

RED_AUTO_TEST_CASE(TestReceive_FastPathServerUpdatePDU3) {
    CryptContext decrypt;

    const char *payload =
/* 0000 */ "\x00\x07\x0a\x02\x00\x00\x00"                                     //.......
        ;
    size_t payload_length = 7;

    GeneratorTransport in_t(payload, payload_length);
    CheckTransport     out_t(payload, payload_length);

    StaticOutStream<65536> out_s;

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fx224(in_t, &end, array_size);
    InStream in_s(array, end - array);
    FastPath::ServerUpdatePDU_Recv in_su(in_s, decrypt, array);

    out_s.out_clear_bytes(FastPath::Update_Send::GetSize(false)); // Fast-Path Update (TS_FP_UPDATE structure) size

    uint8_t updateCode = static_cast<uint8_t>(FastPath::UpdateType::CACHED);

    if (in_su.payload.in_remain()) {
        FastPath::Update_Recv in_upd(in_su.payload, nullptr);

        if (in_upd.updateCode == updateCode) {
            out_s.out_copy_bytes(in_upd.payload.get_data(), in_upd.payload.get_capacity());

            OutStream Upd_s(out_s.get_data(), FastPath::Update_Send::GetSize(false));

            FastPath::Update_Send Upd( Upd_s
                                     , out_s.get_offset() - FastPath::Update_Send::GetSize(false)
                                     , in_upd.updateCode
                                     , in_upd.fragmentation
                                     , 0
                                     , 0
                                     );

            StaticOutStream<256> SvrUpdPDU_s;

            FastPath::ServerUpdatePDU_Send SvrUpdPDU(
                  SvrUpdPDU_s
                , out_s.get_data()
                , out_s.get_offset()
                , in_su.secFlags
                , decrypt
                );

            out_t.send(SvrUpdPDU_s.get_bytes()); // Server Fast-Path Update PDU (TS_FP_UPDATE_PDU)
            out_t.send(out_s.get_bytes());           // Fast-Path Update (TS_FP_UPDATE)
        }
    }

    RED_CHECK_EQUAL(0, in_su.payload.in_remain());
}
