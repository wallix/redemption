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

#include "configs/autogen/enums.hpp"
#include "transport/transport.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <string>
#include <memory>
#include <vector>


class TLSContext;
class ServerNotifier;

class SocketTransport
: public Transport
{
    size_t total_sent = 0;
    size_t total_received = 0;

public:
    int sck;

protected:
    const char * name;

private:
    char ip_address[128];
    int  port;

    std::string * error_message;
    std::unique_ptr<TLSContext> tls;
    enum class TLSState { Uninit, Want, Ok, } tls_state = TLSState::Uninit;

    std::chrono::milliseconds recv_timeout;

    struct AsyncBuf
    {
        std::unique_ptr<uint8_t const> data;
        uint8_t const * p;
        uint8_t const * e;

        AsyncBuf(uint8_t const* data, size_t len);
    };
    std::vector<AsyncBuf> async_buffers;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        dump = 0x100,
    };

    // TODO RZ: We need find a better way to give access of STRAUTHID_AUTH_ERROR_MESSAGE to SocketTransport
    SocketTransport( const char * name, unique_fd sck, const char *ip_address, int port
                   , std::chrono::milliseconds recv_timeout
                   , Verbose verbose, std::string * error_message = nullptr);

    ~SocketTransport() override;

    bool has_waiting_data() const;
    void send_waiting_data();

    bool has_pending_data() const;

    int get_fd() const override { return this->sck; }

    array_view_const_u8 get_public_key() const override;

    void enable_server_tls(const char * certificate_password, const char * ssl_cipher_list) override;

    TlsResult enable_client_tls(bool server_cert_store,
                                ServerCertCheck server_cert_check,
                                ServerNotifier & server_notifier,
                                const char * certif_path
    ) override;

    bool disconnect() override;

    bool connect() override;

protected:
    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    void do_send(const uint8_t * const buffer, size_t len) override;
};
