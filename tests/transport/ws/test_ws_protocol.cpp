/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean, Jonathan Poelen
*/

#include <type_traits>

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "transport/ws/ws_protocol.hpp"

static inline std::string av2str(bytes_view av)
{
    return std::string(av.as_charp(), av.size());
}

RED_AUTO_TEST_CASE(TestWsHttpHeader)
{
    WsHttpHeader ws_http_header;
    using R = WsHttpHeader::State;

    const auto http_header1 =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Protocol: RDP\r\n"
        "Connection: Upgrade\r\n"
        "\r\n"_av;

    RED_CHECK(R::Partial == ws_http_header.extract(""_av));
    RED_CHECK(R::Partial == ws_http_header.extract("a"_av));
    RED_CHECK(R::Partial == ws_http_header.extract("a\r\n"_av));
    RED_CHECK(R::Completed == ws_http_header.extract("a\r\n\r\n"_av));
    RED_CHECK(!ws_http_header.has_key());
    RED_CHECK(ws_http_header.send_header(av2str) == http_header1);

    RED_CHECK(R::Partial == ws_http_header.extract("ds\r\n"_av));
    RED_CHECK(R::Partial == ws_http_header.extract("k: v\r\n"_av));
    RED_CHECK(R::Completed == ws_http_header.extract("k: v\r\n\r\n"_av));
    RED_CHECK(!ws_http_header.has_key());
    RED_CHECK(ws_http_header.send_header(av2str) == http_header1);

    const auto http_header2 =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Protocol: RDP\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: fMtDBfl3gqcXWaZ6erC/gHnUot4=\r\n"
        "\r\n"_av;

    RED_CHECK(R::Partial == ws_http_header.extract("k: v\r\nSec-WebSocket-Key: xyz\r\n"_av));
    RED_CHECK(R::Completed == ws_http_header.extract("k: v\r\nSec-WebSocket-Key: xyz\r\n\r\n"_av));
    RED_CHECK(ws_http_header.has_key());
    RED_CHECK(ws_http_header.send_header(av2str) == http_header2);
}

namespace
{
    template<class T>
    struct ArrayW : T
    {
        bytes_view av() const { return *this; }
    };

    template<class... Ts>
    auto array(Ts... xs)
    {
        using A = std::array<uint8_t, sizeof...(xs)>;
        return ArrayW<A>{A{{uint8_t(xs)...}}};
    }
}

RED_AUTO_TEST_CASE(TestWsParseClient)
{
    using R = ProtocolParseClientResult::State;

    uint8_t s[100] {
        0x80 | 0x02,
        0x80 | 6,
        0xf0, 0x0f, 0x00, 0xff,
        0x2b, 0x2b, 0x2b, 0x2b, 0x85, 0x85,
        0xff
    };
    auto av = [&s](unsigned i) { return writable_u8_array_view{s, i}; };

    RED_CHECK(R::UnsupportedPartialHeader == ws_protocol_parse_client(av(0)).state);
    RED_CHECK(R::UnsupportedPartialHeader == ws_protocol_parse_client(av(1)).state);
    RED_CHECK(R::UnsupportedPartialHeader == ws_protocol_parse_client(av(7)).state);
    RED_CHECK(R::UnsupportedPartialData == ws_protocol_parse_client(av(9)).state);
    uint8_t data_close[] = "\x88\x80\x40\xe9\x27\x4a";
    RED_CHECK(R::Close == ws_protocol_parse_client(make_writable_array_view(data_close)).state);
    ProtocolParseClientResult r;
    RED_CHECK(R::Ok == (r = ws_protocol_parse_client(av(12))).state);
    RED_CHECK(s[12] == 0xff);
    RED_CHECK(r.data == array(0x2b ^ 0xf0, 0x2b ^ 0x0f, 0x2b ^ 0x00, 0x2b ^ 0xff, 0x85 ^ 0xf0, 0x85 ^ 0x0f));

    uint8_t s2[]{
        0x80 | 0x02,
        0x80 | 127,
        1, 0, 0, 0, 0, 0, 0, 0,
        1, 2, 3, 4,
    };
    RED_CHECK(R::Unsupported64BitsPayloadLen == ws_protocol_parse_client(make_writable_array_view(s2)).state);
}

RED_AUTO_TEST_CASE(TestWsSend)
{
    RED_CHECK(ws_protocol_server_send_binary_header(5, av2str) == array(0x80 | 0x02, 5).av());
    RED_CHECK(ws_protocol_server_send_binary_header(125, av2str) == array(0x80 | 0x02, 125).av());
    RED_CHECK(ws_protocol_server_send_binary_header(126, av2str) == array(0x80 | 0x02, 126, 0, 126).av());
    RED_CHECK(ws_protocol_server_send_binary_header(127, av2str) == array(0x80 | 0x02, 126, 0, 127).av());
    RED_CHECK(ws_protocol_server_send_binary_header(229, av2str) == array(0x80 | 0x02, 126, 0, 229).av());
    RED_CHECK(ws_protocol_server_send_binary_header(0xABCD, av2str) == array(0x80 | 0x02, 126, 0xAB, 0xCD).av());
    RED_CHECK(ws_protocol_server_send_binary_header(0x1ABCD, av2str) == array(0x80 | 0x02, 127, 0, 0, 0, 0, 0, 1, 0xAB, 0xCD).av());

    RED_CHECK(ws_protocol_server_send_close_frame(av2str) == array(0x80 | 0x08, 2, 0, 1).av());
}
