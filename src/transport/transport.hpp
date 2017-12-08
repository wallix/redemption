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

#include <sys/time.h>
#include <stdint.h>
#include <cstddef>

#include "cxx/cxx.hpp"
#include "core/error.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/std_stream_proto.hpp"
#include "utils/sugar/byte.hpp"

#include "configs/autogen/enums.hpp"


using std::size_t;

class ServerNotifier;

class Transport : noncopyable
{
protected:
    uint32_t seqno;

public:
    Transport()
    : seqno(0)
    {}

    virtual ~Transport()
    {}

    uint32_t get_seqno() const
    { return this->seqno; }

    enum class TlsResult { Ok, Fail, Want, };
    virtual TlsResult enable_client_tls(
            bool server_cert_store,
            ServerCertCheck server_cert_check,
            ServerNotifier & server_notifier,
            const char * certif_path
        )
    {
        // default enable_tls do nothing
        (void)server_cert_store;
        (void)server_cert_check;
        (void)server_notifier;
        (void)certif_path;
        return TlsResult::Fail;
    }

    virtual void enable_server_tls(const char * certificate_password,
        const char * ssl_cipher_list)
    {
        // default enable_tls do nothing
        (void)certificate_password;
        (void)ssl_cipher_list;
    }

    virtual const uint8_t * get_public_key() const
    {
        return nullptr;
    }

    virtual size_t get_public_key_length() const
    {
        return 0;
    }

    enum class Read : bool { Eof, Ok };

    /// recv_boom read len bytes into buffer or throw an Error
    /// if EOF is encountered at that point it's also an error and
    /// it throws Error(ERR_TRANSPORT_NO_MORE_DATA)
    void recv_boom(byte_ptr buffer, size_t len)
    {
        if (Read::Eof == this->atomic_read(buffer, len)){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
    }

    /// atomic_read either read len bytes into buffer or throw an Error
    /// returned value is either true is read was successful
    /// or false if nothing was read (End of File reached at block frontier)
    /// if an exception is thrown buffer is dirty and may have been modified.
    REDEMPTION_CXX_NODISCARD
    Read atomic_read(byte_ptr buffer, size_t len)
    {
        return this->do_atomic_read(buffer.to_u8p(), len);
    }

    REDEMPTION_CXX_NODISCARD
    Read atomic_read(byte_array buffer)
    {
        return this->do_atomic_read(buffer.to_u8p(), buffer.size());
    }

    REDEMPTION_CXX_NODISCARD
    size_t partial_read(byte_ptr buffer, size_t len)
    {
        return this->do_partial_read(buffer.to_u8p(), len);
    }

    REDEMPTION_CXX_NODISCARD
    size_t partial_read(byte_array buffer)
    {
        return this->do_partial_read(buffer.to_u8p(), buffer.size());
    }

    void send(cbyte_ptr buffer, size_t len)
    {
        this->do_send(buffer.to_u8p(), len);
    }

    void send(cbyte_array buffer)
    {
        this->do_send(buffer.to_u8p(), buffer.size());
    }

    virtual void flush()
    {}

    virtual void seek(int64_t offset, int whence)
    {
        (void)offset;
        (void)whence;
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

private:
    /// Atomic read read exactly the amount of data requested or return an error
    /// @see atomic_read
    virtual Read do_atomic_read(uint8_t * buffer, size_t len) {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }

    virtual size_t do_partial_read(uint8_t * buffer, size_t len) {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }

    virtual void do_send(const uint8_t * buffer, size_t len) {
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

    virtual void timestamp(timeval now)
    { (void)now; }

    /* Some transports are splitted between sequential discrete units
     * (it may be block, chunk, numbered files, directory entries, whatever).
     * Calling next means flushing the current unit and start the next one.
     * seqno countains the current sequence number, starting from 0. */
    virtual bool next()
    {
        this->seqno++;
        return true;
    }

    virtual int get_fd() const { return INVALID_SOCKET; }

private:
    Transport(const Transport &) = delete;
    Transport& operator=(const Transport &) = delete;
};


REDEMPTION_OSTREAM(out, Transport::Read status)
{
    return out << (status == Transport::Read::Ok ? "Read::Ok" : "Read::Eof");
}

struct InTransport
{
    InTransport(Transport & t)
      : t(t)
    {}

    void recv_boom(byte_ptr buffer, size_t len) { this->t.recv_boom(buffer, len); }

    REDEMPTION_CXX_NODISCARD
    Transport::Read atomic_read(byte_ptr buffer, size_t len) { return this->t.atomic_read(buffer, len); }

    REDEMPTION_CXX_NODISCARD
    size_t partial_read(byte_ptr buffer, size_t len) { return this->t.partial_read(buffer, len); }

    uint32_t get_seqno() const { return this->t.get_seqno(); }

    Transport::TlsResult enable_client_tls(
        bool server_cert_store,
        ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier,
        const char * certif_path
    )
    {
        return this->t.enable_client_tls(
            server_cert_store, server_cert_check, server_notifier, certif_path
        );
    }

    void enable_server_tls(const char * certificate_password, const char * ssl_cipher_list)
    { this->t.enable_server_tls(certificate_password, ssl_cipher_list); }

    const uint8_t * get_public_key() const { return this->t.get_public_key(); }
    size_t get_public_key_length() const { return this->t.get_public_key_length(); }

    void seek(int64_t offset, int whence) { this->t.seek(offset, whence); }
    bool disconnect() { return this->t.disconnect(); }
    bool connect() { return this->t.connect(); }
    void timestamp(timeval now) { this->t.timestamp(now); }
    bool next() { return this->t.next(); }
    int get_fd() const { return this->t.get_fd(); }

    REDEMPTION_ATTRIBUTE_DEPRECATED
    Transport & get_transport() const { return this->t; }

private:
    Transport & t;
};

struct OutTransport
{
    OutTransport(Transport & t)
      : t(t)
    {}

    void send(cbyte_ptr buffer, size_t len) { this->t.send(buffer, len); }

    uint32_t get_seqno() const { return this->t.get_seqno(); }

    Transport::TlsResult enable_client_tls(
        bool server_cert_store,
        ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier,
        const char * certif_path
    )
    {
        return this->t.enable_client_tls(
            server_cert_store, server_cert_check, server_notifier, certif_path
        );
    }

    void enable_server_tls(const char * certificate_password, const char * ssl_cipher_list)
    { this->t.enable_server_tls(certificate_password, ssl_cipher_list); }

    const uint8_t * get_public_key() const { return this->t.get_public_key(); }
    size_t get_public_key_length() const { return this->t.get_public_key_length(); }

    void seek(int64_t offset, int whence) { this->t.seek(offset, whence); }
    bool disconnect() { return this->t.disconnect(); }
    bool connect() { return this->t.connect(); }
    void timestamp(timeval now) { this->t.timestamp(now); }
    bool next() { return this->t.next(); }
    int get_fd() const { return this->t.get_fd(); }

    // TODO REDEMPTION_ATTRIBUTE_DEPRECATED
    Transport & get_transport() const { return this->t; }

private:
    Transport & t;
};
