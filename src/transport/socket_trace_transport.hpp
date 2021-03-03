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
   Copyright (C) Wallix 2019
   Author(s): Christophe Grosjean, David Fort

   Transport layer abstraction, Extension on SocketTransport
   with recording traffic abilities
*/

#include "transport/socket_transport.hpp"

struct TraceTransport final : public Transport
{
    using Transport::send;

    TraceTransport(const char * name, SocketTransport & sck_trans)
    : strans(sck_trans)
    , name(name)
        {}

    Transport::Read do_atomic_read(uint8_t * buffer, std::size_t len) override
    {
        LOG_IF(this->enable_trace, LOG_DEBUG, "%s do_atomic_read", this->name);
        return this->strans.atomic_read(buffer, len);
    }

    std::size_t do_partial_read(uint8_t * buffer, std::size_t len) override
    {
        LOG_IF(this->enable_trace, LOG_DEBUG, "%s do_partial_read", this->name);
        return this->strans.partial_read(buffer, len);
    }

public:
    void do_send(const uint8_t * buffer, std::size_t len) override
    {
        LOG_IF(this->enable_trace_send, LOG_DEBUG, "%s do_send", this->name);
        if (this->enable_trace_send){
            hexdump_d({buffer, len});
        }
        this->strans.send(buffer, len);
    }

    [[nodiscard]] int get_fd() const override
    {
        return this->strans.get_fd();
    }

    [[nodiscard]] u8_array_view get_public_key() const override
    {
        return this->strans.get_public_key();
    }

    void enable_server_tls(const char * certificate_password, const char * ssl_cipher_list, uint32_t tls_min_level, uint32_t tls_max_level, bool show_common_cipher_list) override
    {
        return this->strans.enable_server_tls(certificate_password, ssl_cipher_list, tls_min_level, tls_max_level, show_common_cipher_list);
    }

    TlsResult enable_client_tls(ServerNotifier & server_notifier, const TLSClientParams & tls_client_params) override
    {
        return this->strans.enable_client_tls(server_notifier, tls_client_params);
    }

    bool disconnect() override
    {
        return this->strans.disconnect();
    }

    bool connect() override
    {
        return this->strans.connect();
    }

    void set_trace_send(bool send_trace)
    {
        this->enable_trace_send = send_trace;
    }

    void set_trace_receive(bool receive_trace)
    {
        this->enable_trace = receive_trace;
    }

//        private:
    SocketTransport & strans;
    const char * name;

    bool enable_trace = false;
    bool enable_trace_send = false;
};

