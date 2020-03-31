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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/sugar/splitter.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/base64.hpp"
#include "system/ssl_sha1.hpp"

#include <array>
#include <algorithm>
#include <functional>

#include <cstring>

namespace WsHeader
{
    constexpr unsigned Fin = 0x80;
    enum OpCode : uint8_t
    {
        Text = 0x1,
        Binary = 0x2,
        Close = 0x8,
        Ping = 0x9,
        Pong = 0xA,
    };
} // namespace WsHeader


struct WsHttpHeader
{
    std::array<char, 28> base64_key_rep {};

    [[nodiscard]] bool has_key() const noexcept
    {
        return this->base64_key_rep[0];
    }

    enum class State : bool
    {
        Partial,
        Completed,
    };

    State extract(array_view_const_char in) noexcept
    {
        if (in.size() < 5
         || in[in.size()-4] != '\r'
         || in[in.size()-3] != '\n'
         || in[in.size()-2] != '\r'
         || in[in.size()-1] != '\n'
        ) {
            return State::Partial;
        }

        auto extract_http_header_value = [&](array_view_const_char needle, auto cb){
            auto it = std::search(in.begin(), in.end(), needle.begin(), needle.end());
            if (it != in.end()) {
                it += needle.size();
                auto e = it;
                while (*++e != '\r') {
                }
                cb({it, e});
            }
        };

        extract_http_header_value("\r\nSec-WebSocket-Key: "_av, [&](array_view_const_char value){
            constexpr auto WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"_av;
            SslSha1 sha1;
            uint8_t digest[SslSha1::DIGEST_LENGTH];
            sha1.update(value);
            sha1.update(WS_GUID);
            sha1.final(digest);
            static_assert(base64_encode_size(sizeof(digest)) == sizeof(this->base64_key_rep));
            base64_encode(make_array_view(digest), this->base64_key_rep);
        });

        return State::Completed;
    }

    template<class F>
    decltype(auto) send_header(F&& f)
    {
        if(this->has_key()) {
            char http_header_buf[] =
                "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Sec-WebSocket-Protocol: RDP\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: 1234567890123456789012345678\r\n"
                "\r\n";
            auto http_header = make_array_view(http_header_buf, std::size(http_header_buf)-1);
            std::memcpy(
                http_header_buf + http_header.size() - (this->base64_key_rep.size() + 4u),
                this->base64_key_rep.data(),
                this->base64_key_rep.size());
            return std::invoke(static_cast<F&&>(f), http_header);
        }

        auto http_header =
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Sec-WebSocket-Protocol: RDP\r\n"
            "Connection: Upgrade\r\n"
            "\r\n"_av;
        return std::invoke(static_cast<F&&>(f), http_header);
    }
};

struct ProtocolParseClientResult
{
    enum class State : uint8_t
    {
        Ok,
        Close,
        UnsupportedPartialHeader,
        Unsupported64BitsPayloadLen,
        UnsupportedPartialData,
    };
    State state;
    writable_bytes_view data;
};

/*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    |     Extended payload length continued, if payload len == 127  |
    + - - - - - - - - - - - - - - - +-------------------------------+
    |                               |Masking-key, if MASK set to 1  |
    +-------------------------------+-------------------------------+
    | Masking-key (continued)       |          Payload Data         |
    +-------------------------------- - - - - - - - - - - - - - - - +
    :                     Payload Data continued ...                :
    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    |                     Payload Data continued ...                |
    +---------------------------------------------------------------+

    client: Mask = 1

    minimal header size = 6  (fin + opcode + payload_len)
    maximal header size = 16 (fin + opcode + payload_len + extended_payload + extended_payload_continuastion)
    minimal supported header size = 8 ( 6bits isn't supported because maximal packet to RDP is equal to 16bits)
*/
inline ProtocolParseClientResult ws_protocol_parse_client(writable_bytes_view data) noexcept
{
    if (data.size() < 8) {
        return {
            (not data.empty() && (data[0] & 0xF) == WsHeader::OpCode::Close)
             ? ProtocolParseClientResult::State::Close
             : ProtocolParseClientResult::State::UnsupportedPartialHeader,
            writable_bytes_view{}
        };
    }

    uint8_t const opcode = data[0] & 0xF;
    [[maybe_unused]] bool const fin = (data[0] >> 7) & 0x01;
    [[maybe_unused]] bool const masked = (data[1] >> 7) & 0x01;
    uint8_t const length_field = data[1] & (~0x80);

    if (opcode == WsHeader::OpCode::Close) {
        return {ProtocolParseClientResult::State::Close, writable_bytes_view{}};
    }

    assert(opcode == WsHeader::Binary);
    assert(fin);
    assert(masked);

    size_t payload_length = 0;
    size_t pos = 2;

    if (length_field <= 125u) {
        payload_length = length_field;
    }
    else if (length_field == 126u) { // msglen is 16bit!
        payload_length = (
            (data[2] << 8) |
            (data[3])
        );
        pos += 2;
    }
    else /*if(length_field == 127u)*/ { // msglen is 64bit!
        return {ProtocolParseClientResult::State::Unsupported64BitsPayloadLen, writable_bytes_view{}};
        // static_assert(sizeof(size_t) < sizeof(uint64_t));
        // payload_length = (
        //     (uint64_t(data[2]) << 56) |
        //     (uint64_t(data[3]) << 48) |
        //     (uint64_t(data[4]) << 40) |
        //     (uint64_t(data[5]) << 32) |
        //     (uint64_t(data[6]) << 24) |
        //     (uint64_t(data[7]) << 16) |
        //     (uint64_t(data[8]) << 8 ) |
        //     (uint64_t(data[9]))
        // );
        // pos += 8;
    }

    uint8_t mask[4] {
        data[pos  ],
        data[pos+1],
        data[pos+2],
        data[pos+3],
    };
    pos += 4;

    if(data.size() - pos < payload_length) {
        return {ProtocolParseClientResult::State::UnsupportedPartialData, writable_bytes_view{}};
    }

    auto first = data.begin() + pos;
    auto end = first + (payload_length - payload_length % 4u);
    while (first < end) {
        *first++ ^= mask[0];
        *first++ ^= mask[1];
        *first++ ^= mask[2];
        *first++ ^= mask[3];
    }
    switch (payload_length % 4) {
        case 3: first[2] ^= mask[2]; [[fallthrough]];
        case 2: first[1] ^= mask[1]; [[fallthrough]];
        case 1: first[0] ^= mask[0]; [[fallthrough]];
        case 0: break;
    }

    return {ProtocolParseClientResult::State::Ok, data.subarray(pos, payload_length)};
}

template<class F>
decltype(auto) ws_protocol_server_send_binary_header(uint64_t const len, F&& f)
{
    constexpr uint8_t binary_frame = WsHeader::Fin | WsHeader::OpCode::Binary;

    if (len <= 125) {
        uint8_t buf[] = {binary_frame, uint8_t(len)};
        return std::invoke(static_cast<F&&>(f), make_array_view(buf));
    }

    if (len <= 0xffff) {
        uint8_t buf[] = {binary_frame, 126, uint8_t(len >> 8), uint8_t(len)};
        return std::invoke(static_cast<F&&>(f), make_array_view(buf));
    }

    {
        uint8_t buf[] = {
            binary_frame,
            127,
            uint8_t(len >> 56),
            uint8_t(len >> 48),
            uint8_t(len >> 40),
            uint8_t(len >> 32),
            uint8_t(len >> 24),
            uint8_t(len >> 16),
            uint8_t(len >> 8),
            uint8_t(len),
        };
        return std::invoke(static_cast<F&&>(f), make_array_view(buf));
    }
}

template<class F>
decltype(auto) ws_protocol_server_send_close_frame(F&& f)
{
    uint8_t buf[]{
        WsHeader::Fin | WsHeader::OpCode::Close,
        2,
        0,
        1,
    };
    return std::invoke(static_cast<F&&>(f), make_array_view(buf));
}
