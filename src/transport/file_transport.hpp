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

class FileTransport : noncopyable
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
    FileTransport()
    : total_received(0)
    , total_sent(0)
    , seqno(0)
    , last_quantum_received(0)
    , last_quantum_sent(0)
    , status(true)
    , authentifier(get_null_authentifier())
    {}

    virtual ~FileTransport()
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

    void send(const char * const buffer, size_t len)
    {
        this->do_send(reinterpret_cast<const uint8_t * const>(buffer), len);
    }

    int recv(char * buffer, size_t len)
    {
        return this->do_recv(reinterpret_cast<uint8_t*>(buffer), len);
    }

    int recv(uint8_t * buffer, size_t len)
    {
        return this->do_recv(buffer, len);
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

    virtual int do_recv(uint8_t * buffer, size_t len) {
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
    FileTransport(const FileTransport &) = delete;
    FileTransport& operator=(const FileTransport &) = delete;
};
