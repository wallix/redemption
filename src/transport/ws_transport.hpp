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

#pragma once

#include <chrono>

#include "transport/socket_transport.hpp"

class WsTransport
: public SocketTransport
{
public:
    WsTransport(
        const char * name, unique_fd sck, const char *ip_address, int port,
        std::chrono::milliseconds recv_timeout, Verbose verbose,
        std::string * error_message = nullptr);

    bool disconnect() override;

protected:
    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    void do_send(const uint8_t * const buffer, size_t len) override;

    TlsResult enable_client_tls(bool server_cert_store,
                                ServerCertCheck server_cert_check,
                                ServerNotifier & server_notifier,
                                const char * certif_path
    ) override;

private:
    class D;

    enum class State
    {
        HttpHeader,
        Ws,
        Closed,
        Error,
    };

    State state = State::HttpHeader;
};
