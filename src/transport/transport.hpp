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

#include "utils/log.hpp"
#include "core/error.hpp"
#include "acl/auth_api.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/invalid_socket.hpp"

#include "configs/autogen/enums.hpp"

#include "core/server_notifier_api.hpp"

using std::size_t;

class Transport : noncopyable
{
    uint64_t total_received;
    uint64_t total_sent;

protected:
    uint32_t seqno;

    uint64_t last_quantum_received;
    uint64_t last_quantum_sent;

    bool status;

    auth_api * authentifier;

public:
    Transport()
    : total_received(0)
    , total_sent(0)
    , seqno(0)
    , last_quantum_received(0)
    , last_quantum_sent(0)
    , status(true)
    , authentifier(get_null_authentifier())
    {}

    virtual ~Transport()
    {}

    uint32_t get_seqno() const
    { return this->seqno; }

    uint64_t get_total_received() const
    { return this->total_received + this->last_quantum_received; }

    uint64_t get_last_quantum_received() const
    { return this->last_quantum_received; }

    uint64_t get_total_sent() const
    { return this->total_sent + this->last_quantum_sent; }

    uint64_t get_last_quantum_sent() const
    { return this->last_quantum_sent; }

    virtual bool get_status() const
    { return this->status; }

    void set_authentifier(auth_api * authentifier)
    {
        this->authentifier = authentifier;
    }

    //void reset_quantum_sent() noexcept
    //{
    //    this->last_quantum_sent = 0;
    //}

    void tick()
    {
        this->total_received += this->last_quantum_received;
        this->total_sent += this->last_quantum_sent;
        this->last_quantum_received = 0;
        this->last_quantum_sent = 0;
    }

    virtual void enable_client_tls(
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

//     void recv(char ** pbuffer, size_t len) __attribute__((deprecated))
//     {
//         this->do_recv(reinterpret_cast<uint8_t **>(pbuffer), len);
//     }

    void send(const char * const buffer, size_t len)
    {
        this->do_send(reinterpret_cast<const uint8_t * const>(buffer), len);
    }

//     void recv(uint8_t ** pbuffer, size_t len) __attribute__((deprecated))
//     {
//         this->do_recv(pbuffer, len);
//     }

    void recv_new(char * buffer, size_t len)
    {
        return this->do_recv_new(reinterpret_cast<uint8_t*>(buffer), len);
    }

    void recv_new(uint8_t * buffer, size_t len)
    {
        return this->do_recv_new(buffer, len);
    }

    void send(const uint8_t * const buffer, size_t len)
    {
        this->do_send(buffer, len);
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
//     virtual void do_recv(uint8_t ** pbuffer, size_t len) __attribute__((deprecated))
//     {
//         (void)pbuffer;
//         (void)len;
//         throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
//     }

    virtual void do_recv_new(uint8_t * buffer, size_t len) {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }


    virtual void do_send(const uint8_t * buffer, size_t len) {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
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

    virtual void request_full_cleaning()
    {}

    virtual int get_fd() const { return INVALID_SOCKET; }

private:
    Transport(const Transport &) = delete;
    Transport& operator=(const Transport &) = delete;
};
