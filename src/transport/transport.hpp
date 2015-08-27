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

#ifndef REDEMPTION_TRANSPORT_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_TRANSPORT_HPP

#include <sys/time.h>
#include <stdint.h>
#include <cstddef>

#include "log.hpp"
#include "error.hpp"
#include "stream.hpp"
#include "auth_api.hpp"
#include "noncopyable.hpp"

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

    uint32_t verbose;

public:
    Transport()
    : total_received(0)
    , total_sent(0)
    , seqno(0)
    , last_quantum_received(0)
    , last_quantum_sent(0)
    , status(true)
    , authentifier(get_null_authentifier())
    , verbose(0)
    {}

    virtual ~Transport()
    {}

    //virtual const SequenceGenerator * seqgen() const noexcept
    //{ return this->pseq; }

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

    bool get_status() const
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

    virtual void enable_client_tls(bool ignore_certificate_change)
    {
        // default enable_tls do nothing
    }

    virtual void enable_server_tls(const char * certificate_password)
    {
        // default enable_tls do nothing
    }

    virtual const uint8_t * get_public_key() const
    {
        return nullptr;
    }

    virtual size_t get_public_key_length() const
    {
        return 0;
    }

    void recv(char ** pbuffer, size_t len)
    {
        this->do_recv(pbuffer, len);
    }

    void send(const char * const buffer, size_t len)
    {
        this->do_send(buffer, len);
    }

    void recv(uint8_t ** pbuffer, size_t len)
    {
        this->do_recv(reinterpret_cast<char **>(pbuffer), len);
    }

    void send(const uint8_t * const buffer, size_t len)
    {
        this->do_send(reinterpret_cast<const char * const>(buffer), len);
    }

    virtual void flush()
    {}

    virtual void seek(int64_t offset, int whence)
    {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

private:
    virtual void do_recv(char ** pbuffer, size_t len) {
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }

    virtual void do_send(const char * const buffer, size_t len) {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }

public:

    TODO("All these functions should be changed after Stream refactoring to remove dependency between transport and Stream")

    void send(Stream const & header1, Stream const & header2, Stream const & header3, HStream & stream)
    {
        stream.copy_to_head(header3.get_data(), header3.size());
        stream.copy_to_head(header2.get_data(), header2.size());
        stream.copy_to_head(header1.get_data(), header1.size());
        this->send(stream);
    }

    void send(Stream const & header1, Stream const & header2, HStream & stream)
    {
        stream.copy_to_head(header2.get_data(), header2.size());
        stream.copy_to_head(header1.get_data(), header1.size());
        this->send(stream.get_data(), stream.size());
    }

    void send(Stream const & header, HStream & stream)
    {
        stream.copy_to_head(header.get_data(), header.size());
        this->send(stream.get_data(), stream.size());
    }

    void send(Stream const & stream)
    {
        this->send(stream.get_data(), stream.size());
    }

    void send(InStream const & stream)
    {
        this->send(stream.get_data(), stream.get_capacity());
    }


    virtual bool disconnect()
    {
        return true;
    }

    virtual bool connect()
    {
        return true;
    }

    virtual void timestamp(timeval now)
    {}

    virtual bool next()
    REDOC("Some transports are splitted between sequential discrete units"
          "(it may be block, chunk, numbered files, directory entries, whatever)."
          "Calling next means flushing the current unit and start the next one."
          "seqno countains the current sequence number, starting from 0.")
    {
        this->seqno++;
        return true;
    }

    virtual void request_full_cleaning()
    {}

private:
    Transport(const Transport &) = delete;
    Transport& operator=(const Transport &) = delete;
};

#endif
