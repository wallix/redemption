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
#include "transport/ws/ws_protocol.hpp"
#include "utils/log.hpp"

#include <cstring>


struct WsTransport::D
{
    static auto sender(WsTransport& t)
    {
        return [&t](const_bytes_view data) {
            t.SocketTransport::do_send(data.data(), data.size());
        };
    }
};

WsTransport::WsTransport(
    const char * name, unique_fd sck, const char *ip_address, int port,
    std::chrono::milliseconds recv_timeout, Verbose verbose,
    std::string * error_message)
: SocketTransport(name, std::move(sck), ip_address, port, recv_timeout, verbose, error_message)
{}

size_t WsTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    size_t res = SocketTransport::do_partial_read(buffer, len);

    switch (this->state)
    {
    case State::HttpHeader: {
        WsHttpHeader http_header;
        if (WsHttpHeader::State::Completed
            != http_header.extract({char_ptr_cast(buffer), res})
        ) {
            this->state = State::Error;
            LOG(LOG_ERR, "partial header");
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        http_header.send_header(D::sender(*this));

        this->state = State::Ws;
        return 0;
    }

    case State::Ws: {
        array_view_u8 data{buffer, res};
        res = 0;

        while (!data.empty()) {
            auto frame = ws_protocol_parse_client(data);
            using Parse = ProtocolParseClientResult::State;
            switch (frame.state)
            {
            case Parse::UnsupportedPartialHeader:
                LOG(LOG_ERR, "WebSocket: partial header isn't supported");
                throw Error(ERR_TRANSPORT_READ_FAILED);

            case Parse::UnsupportedPartialData:
                LOG(LOG_ERR, "WebSocket: partial data isn't supported");
                throw Error(ERR_TRANSPORT_READ_FAILED);

            case Parse::Unsupported64BitsPayloadLen:
                LOG(LOG_ERR, "WebSocket: payload_length too great");
                throw Error(ERR_TRANSPORT_READ_FAILED);

            case Parse::Close:
                this->disconnect();
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);

            case Parse::Ok:
                memmove(buffer + res, frame.data.data(), frame.data.size());
                res += frame.data.size();
                data = {frame.data.end(), data.end()};
            }
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
    ws_protocol_server_send_binary_header(len, D::sender(*this));
    SocketTransport::do_send(buffer, len);
}

WsTransport::TlsResult WsTransport::enable_client_tls(
    bool /*server_cert_store*/, ServerCertCheck /*server_cert_check*/,
    ServerNotifier & /*server_notifier*/, const char * /*certif_path*/)
{
    LOG(LOG_DEBUG, "enable_client_tls");
    return TlsResult::Fail;
}

bool WsTransport::disconnect()
{
    this->state = State::Closed;

    if (this->sck != INVALID_SOCKET) {
        ws_protocol_server_send_close_frame(D::sender(*this));
        return SocketTransport::disconnect();
    }

    return false;
}
