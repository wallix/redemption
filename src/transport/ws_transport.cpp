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
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

   Transport layer abstraction, socket implementation with TLS support
*/

#include "transport/ws_transport.hpp"
#include "utils/hexdump.hpp"
#include "utils/sugar/flags.hpp"
#include "utils/log.hpp"
#include "utils/sugar/splitter.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/base64.hpp"
#include "system/ssl_sha1.hpp"

#include <cstring>
#include <algorithm>
#include <functional>

namespace
{
    enum class WsOption
    {
        PermessageDeflate,
        EncodingGZip,
        EncodingDeflate,
        COUNT_,
    };

    enum class State
    {
        HttpHeader,
        Ws,
        Closed,
        Error,
    };

    struct MiniBuf
    {
        char data[24];
        uint8_t len = 0;
    };

    constexpr auto WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"_av;
} // anonymous namespace

template<>
struct utils::enum_as_flag<WsOption>
{
    static constexpr std::size_t max = std::size_t(WsOption::COUNT_);
};

struct WsTransport::D
{
    State state = State::HttpHeader;
    utils::flags_t<WsOption> ws_options;
    std::array<char, 28> base64_key_rep {};
    MiniBuf minibuf;

    void extract_http_header(array_view_const_char in)
    {
        auto extract_http_header_value = [&](array_view_const_char needle, auto cb){
            auto it = std::search(in.begin(), in.end(),
                std::boyer_moore_searcher(needle.begin(), needle.end()));
            if (it != in.end()) {
                it += needle.size();
                auto e = it;
                while (*++e != '\r') {
                }
                cb({it, e});
            }
        };

        auto aveq = [](array_view_const_char a, array_view_const_char b){
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
        };

        extract_http_header_value("\r\nAccept-Encoding: "_av, [&](array_view_const_char value){
            for (auto word : get_split(value.begin(), value.end(), ',')) {
                auto word_begin = ltrim(word.begin(), word.end());
                struct Data {
                    array_view_const_char str;
                    WsOption option;
                };
                for (auto data : {
                    Data{"gzip"_av, WsOption::EncodingGZip},
                    Data{"deflate"_av, WsOption::EncodingDeflate}
                }) {
                    if (aveq(data.str, {word_begin, word.end()})) {
                        this->ws_options += data.option;
                        break;
                    }
                }
            }
        });

        extract_http_header_value("\r\nSec-WebSocket-Key: "_av, [&](array_view_const_char value){
            SslSha1 sha1;
            uint8_t digest[SslSha1::DIGEST_LENGTH];
            sha1.update(value);
            sha1.update(WS_GUID);
            sha1.final(digest);
            base64_encode(make_array_view(digest), this->base64_key_rep);
        });

        extract_http_header_value("\r\nSec-WebSocket-Extensions: "_av, [&](array_view_const_char value){
            if (aveq(value, "permessage-deflate"_av)) {
                this->ws_options += WsOption::PermessageDeflate;
            }
        });
    }
};

WsTransport::WsTransport(
    const char * name, unique_fd sck, const char *ip_address, int port,
    std::chrono::milliseconds recv_timeout, Verbose verbose,
    std::string * error_message)
: SocketTransport(name, std::move(sck), ip_address, port, recv_timeout, verbose, error_message)
, d(std::make_unique<D>())
{}

WsTransport::~WsTransport() = default;

#if 0
void WsTransport::do_send(const uint8_t * const buffer, size_t const len)
{
    SocketTransport::do_send(buffer, len);
}

WsTransport::Read WsTransport::do_atomic_read(uint8_t * buffer, size_t len)
{
    return SocketTransport::do_atomic_read(buffer, len);
}

size_t WsTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    return SocketTransport::do_partial_read(buffer, len);
}
#else

namespace
{
    constexpr unsigned Fin = 0x80;
    enum WsOpCode : uint8_t
    {
        Text = 0x1,
        Binary = 0x2,
        Close = 0x8,
        Ping = 0x9,
        Pong = 0xA,
    };
}

size_t WsTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    size_t res = SocketTransport::do_partial_read(buffer, len);

    switch (this->d->state) {
        case State::HttpHeader: {
            const array_view_const_char in{char_ptr_cast(buffer), res};
            if (in.size() < 5
             || in[in.size()-4] != '\r'
             || in[in.size()-3] != '\n'
             || in[in.size()-2] != '\r'
             || in[in.size()-1] != '\n'
            ) {
                this->d->state = State::Error;
                LOG(LOG_ERR, "partial header");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }

            this->d->extract_http_header(in);

            if(this->d->base64_key_rep[0]) {
                char http_header_buf[] =
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Sec-WebSocket-Protocol: RDP\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: 1234567890123456789012345678\r\n"
                    "\r\n";
                auto http_header = make_array_view(http_header_buf, std::size(http_header_buf)-1);
                std::memcpy(
                    http_header_buf + http_header.size() - (this->d->base64_key_rep.size() + 4u),
                    this->d->base64_key_rep.data(),
                    this->d->base64_key_rep.size());
                SocketTransport::do_send(byte_ptr_cast(http_header.data()), http_header.size());
            }
            else {
                auto http_header =
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Sec-WebSocket-Protocol: RDP\r\n"
                    "Connection: Upgrade\r\n"
                    "\r\n"_av;
                SocketTransport::do_send(byte_ptr_cast(http_header.data()), http_header.size());
            }

            this->d->state = State::Ws;
            return 0;
        }

        case State::Ws: {
            array_view_u8 in{buffer, res};
            res = 0;

            while (!in.empty()) {
                if (in.size() < 12) {
                    LOG(LOG_ERR, "WebSocket: partial header isn't supported");
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }

                int const opcode = in[0] & 0xF;
                [[maybe_unused]] bool const fin = (in[0] >> 7) & 0x01;
                [[maybe_unused]] bool const masked = (in[1] >> 7) & 0x01;
                uint8_t const length_field = in[1] & (~0x80);

                if (opcode == WsOpCode::Close) {
                    this->disconnect();
                    throw Error(ERR_TRANSPORT_NO_MORE_DATA);
                }

                assert(opcode == int(WsOpCode::Binary));
                assert(fin);
                assert(masked);

                size_t payload_length = 0;
                size_t pos = 2;

                if (length_field <= 125u) {
                    payload_length = length_field;
                }
                else if (length_field == 126u) { // msglen is 16bit!
                    payload_length = (
                        (in[2] << 8) |
                        (in[3])
                    );
                    pos += 2;
                }
                else /*if(length_field == 127u)*/ { // msglen is 64bit!
                    LOG(LOG_ERR, "WebSocket: payload_length too great");
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                    // static_assert(sizeof(size_t) < sizeof(uint64_t));
                    // payload_length = (
                    //     (uint64_t(in[2]) << 56) |
                    //     (uint64_t(in[3]) << 48) |
                    //     (uint64_t(in[4]) << 40) |
                    //     (uint64_t(in[5]) << 32) |
                    //     (uint64_t(in[6]) << 24) |
                    //     (uint64_t(in[7]) << 16) |
                    //     (uint64_t(in[8]) << 8 ) |
                    //     (uint64_t(in[9]))
                    // );
                    // pos += 8;
                }

                uint8_t mask[4] {
                    in[pos  ],
                    in[pos+1],
                    in[pos+2],
                    in[pos+3],
                };
                pos += 4;

                if(in.size() - pos < payload_length) {
                    LOG(LOG_ERR, "WebSocket: partial data isn't supported");
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }

                auto first = in.begin() + pos;
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

                memmove(buffer + res, in.data() + pos, payload_length);
                res += payload_length;
                in = in.array_from_offset(pos + payload_length);
            }

            break;
        }

        case State::Closed:
            throw Error(ERR_TRANSPORT_CLOSED);
        case State::Error:
            throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    return res;
}

WsTransport::Read WsTransport::do_atomic_read(uint8_t* /*buffer*/, size_t /*len*/)
{
    LOG(LOG_ERR, "WebSocket: do_atomic_read isn't supported");
    throw Error(ERR_TRANSPORT_READ_FAILED);
}

void WsTransport::do_send(const uint8_t * const buffer, size_t const len)
{
    constexpr uint8_t binary_frame = Fin | WsOpCode::Binary;

    if (len <= 125) {
        uint8_t buf[] = {binary_frame, uint8_t(len)};
        SocketTransport::do_send(buf, std::size(buf));
    }
    else if (len <= 0xffff) {
        uint8_t buf[] = {binary_frame, 126, uint8_t(len >> 8), uint8_t(len)};
        SocketTransport::do_send(buf, std::size(buf));
    }
    else {
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
        SocketTransport::do_send(buf, std::size(buf));
    }

    SocketTransport::do_send(buffer, len);
}

#endif

WsTransport::TlsResult WsTransport::enable_client_tls(
    bool /*server_cert_store*/, ServerCertCheck /*server_cert_check*/,
    ServerNotifier & /*server_notifier*/, const char * /*certif_path*/)
{
    LOG(LOG_DEBUG, "enable_client_tls");
    return TlsResult::Fail;
}

bool WsTransport::disconnect()
{
    this->d->state = State::Closed;

    if (this->sck != INVALID_SOCKET) {
        uint8_t data[]{
            Fin | WsOpCode::Close,
            2,
            0,
            1,
        };
        SocketTransport::do_send(data, std::size(data));
        return SocketTransport::disconnect();
    }

    return false;
}
