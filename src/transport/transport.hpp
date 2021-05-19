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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Transport layer abstraction
*/


#pragma once

#include <cstdint>
#include <cstddef>

#include "cxx/cxx.hpp"
#include "core/error.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/log.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/buffer_view.hpp"


class ServerNotifier;


struct TLSClientParams
{
    uint32_t tls_min_level = 0;
    uint32_t tls_max_level = 0;
    bool show_common_cipher_list = false;
    std::string cipher_string;
    int security_level = 1;
    bool anonymous_tls = false;
};


class Transport : noncopyable
{
public:
    explicit Transport() = default;

    Transport(const Transport &) = delete;
    Transport& operator=(const Transport &) = delete;

    virtual ~Transport() = default;

    enum class [[nodiscard]] TlsResult : uint8_t { Ok, Fail, Want, WaitExternalEvent, };
    virtual TlsResult enable_client_tls(
        ServerNotifier & server_notifier, const TLSClientParams & /*tls_client_params*/)
    {
        // default enable_tls do nothing
        (void)server_notifier;
        return TlsResult::Fail;
    }

    virtual void enable_server_tls(const char * certificate_password,
        const char * ssl_cipher_list, uint32_t tls_min_level, uint32_t tls_max_level, bool show_common_cipher_list)
    {
        // default enable_tls do nothing
        (void)certificate_password;
        (void)ssl_cipher_list;
        (void)tls_min_level;
        (void)tls_max_level;
        (void)show_common_cipher_list;
    }

    [[nodiscard]] virtual u8_array_view get_public_key() const
    {
        return {};
    }

    enum class [[nodiscard]] Read : bool { Eof, Ok };

    /// recv_boom read len bytes into buffer or throw an Error
    /// if EOF is encountered at that point it's also an error and
    /// it throws Error(ERR_TRANSPORT_NO_MORE_DATA)
    writable_bytes_view recv_boom(writable_byte_ptr buffer, size_t len)
    {
        if (Read::Eof == this->atomic_read(buffer, len)) {
            LOG(LOG_ERR, "Transport::recv_boom (1): Failed to read transport!");
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        return {buffer, len};
    }

    writable_bytes_view recv_boom(writable_buffer_view buffer)
    {
        if (Read::Eof == this->atomic_read(buffer.as_u8p(), buffer.size())) {
            LOG(LOG_ERR, "Transport::recv_boom (2): Failed to read transport!");
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        return buffer;
    }

    /// atomic_read either read len bytes into buffer or throw an Error
    /// returned value is either true is read was successful
    /// or false if nothing was read (End of File reached at block frontier)
    /// if an exception is thrown buffer is dirty and may have been modified.
    [[nodiscard]]
    Read atomic_read(writable_byte_ptr buffer, size_t len)
    {
        return this->do_atomic_read(buffer.as_u8p(), len);
    }

    [[nodiscard]]
    Read atomic_read(writable_buffer_view buffer)
    {
        return this->do_atomic_read(buffer.as_u8p(), buffer.size());
    }

    // TODO returns writable_bytes_view
    [[nodiscard]]
    size_t partial_read(writable_byte_ptr buffer, size_t len)
    {
        return this->do_partial_read(buffer.as_u8p(), len);
    }

    // TODO returns writable_bytes_view
    [[nodiscard]]
    size_t partial_read(writable_buffer_view buffer)
    {
        return this->do_partial_read(buffer.as_u8p(), buffer.size());
    }

    void send(byte_ptr buffer, size_t len)
    {
        this->do_send(buffer.as_u8p(), len);
    }

    void send(bytes_view buffer)
    {
        this->do_send(buffer.as_u8p(), buffer.size());
    }

    virtual void flush()
    {}

private:
    // note: `func(buf, len)` derived function shadows `f(array)` function base.
    // do_*() function resolves this problem

    /// Atomic read read exactly the amount of data requested or return an error
    /// @see atomic_read
    virtual Read do_atomic_read(uint8_t * buffer, size_t len)
    {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }

    virtual size_t do_partial_read(uint8_t * buffer, size_t len)
    {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }

    virtual void do_send(const uint8_t * buffer, size_t len)
    {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }

public:
    virtual bool disconnect()
    {
        return true;
    }

    virtual bool connect()
    {
        return true;
    }

    /* Some transports are splitted between sequential discrete units
     * (it may be block, chunk, numbered files, directory entries, whatever).
     * Calling next means flushing the current unit and start the next one.
     * seqno countains the current sequence number, starting from 0. */
    virtual bool next()
    {
        return true;
    }

    [[nodiscard]] virtual int get_fd() const { return INVALID_SOCKET; }
};


struct InTransport
{
    InTransport(Transport & t)
      : t(t)
    {}

    void recv_boom(writable_byte_ptr buffer, size_t len) { this->t.recv_boom(buffer, len); }
    void recv_boom(writable_buffer_view buffer) { this->t.recv_boom(buffer); }

    [[nodiscard]]
    Transport::Read atomic_read(writable_byte_ptr buffer, size_t len) { return this->t.atomic_read(buffer, len); }

    [[nodiscard]]
    size_t partial_read(writable_byte_ptr buffer, size_t len) { return this->t.partial_read(buffer, len); }

    Transport::TlsResult enable_client_tls(ServerNotifier & server_notifier, const TLSClientParams & tls_client_params)
    {
        return this->t.enable_client_tls(server_notifier, tls_client_params);
    }

    void enable_server_tls(const char * certificate_password, const char * ssl_cipher_list, uint32_t tls_min_level, uint32_t tls_max_level, bool show_common_cipher_list)
    { this->t.enable_server_tls(certificate_password, ssl_cipher_list, tls_min_level, tls_max_level, show_common_cipher_list); }

    [[nodiscard]] u8_array_view get_public_key() const { return this->t.get_public_key(); }

    bool disconnect() { return this->t.disconnect(); }
    bool connect() { return this->t.connect(); }
    bool next() { return this->t.next(); }
    [[nodiscard]] int get_fd() const { return this->t.get_fd(); }

private:
    Transport & t;
};

struct OutTransport
{
    OutTransport(Transport & t)
      : t(t)
    {}

    void send(byte_ptr buffer, size_t len) { this->t.send(buffer, len); }
    void send(bytes_view buffer) { this->t.send(buffer); }

    Transport::TlsResult enable_client_tls(ServerNotifier & server_notifier, const TLSClientParams & tls_client_params)
    {
        return this->t.enable_client_tls(server_notifier, tls_client_params);
    }

    void enable_server_tls(const char * certificate_password, const char * ssl_cipher_list, uint32_t tls_min_level, uint32_t tls_max_level, bool show_common_cipher_list)
    { this->t.enable_server_tls(certificate_password, ssl_cipher_list, tls_min_level, tls_max_level, show_common_cipher_list); }

    [[nodiscard]] u8_array_view get_public_key() const { return this->t.get_public_key(); }

    bool disconnect() { return this->t.disconnect(); }
    bool connect() { return this->t.connect(); }
    bool next() { return this->t.next(); }
    [[nodiscard]] int get_fd() const { return this->t.get_fd(); }

    // TODO [[deprecated]]
    [[nodiscard]] Transport & get_transport() const { return this->t; }

private:
    Transport & t;
};
