/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Transport layer abstraction
*/

#ifndef _REDEMPTION_TRANSPORT_TRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_TRANSPORT_HPP_

#include <sys/types.h> // recv, send
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include </usr/include/openssl/ssl.h>
#include </usr/include/openssl/err.h>

#include "error.hpp"
#include "log.hpp"
#include "fileutils.hpp"
#include "netutils.hpp"
#include "rio/rio.h"
#include "rio/rio_impl.h"
#include "stream.hpp"


class Transport {
public:
    timeval future;
    uint32_t seqno;
    uint64_t total_received;
    uint64_t last_quantum_received;
    uint64_t total_sent;
    uint64_t last_quantum_sent;
    uint64_t quantum_count;
    bool status;

    Transport() :
        seqno(0),
        total_received(0),
        last_quantum_received(0),
        total_sent(0),
        last_quantum_sent(0),
        quantum_count(0),
        status(true)
    {}

    virtual ~Transport() 
    {
    }

    void tick() {
        quantum_count++;
        last_quantum_received = 0;
        last_quantum_sent = 0;
    }

    virtual void enable_tls()
    {
        // default enable_tls do nothing
    }

    void recv(uint8_t ** pbuffer, size_t len) throw (Error) {
        this->recv(reinterpret_cast<char **>(pbuffer), len);
    }
    virtual void recv(char ** pbuffer, size_t len) throw (Error) = 0;
    virtual void send(const char * const buffer, size_t len) throw (Error) = 0;
    void send(Stream & stream) throw(Error) {
        this->send(stream.data, stream.size());
    }
    void send(const uint8_t * const buffer, size_t len) throw (Error) {
        this->send(reinterpret_cast<const char * const>(buffer), len);
    }
    virtual void disconnect(){}
    virtual bool connect()
    {
        return true;
    }

    virtual void flush()
    {
    }

    virtual void timestamp(timeval now)
    {
        this->future = now;
    }

    virtual bool next()
    REDOC("Some transports are splitted between sequential discrete units"
          "(it may be block, chunk, numbered files, directory entries, whatever)."
          "Calling next means flushing the current unit and start the next one."
          "seqno countains the current sequence number, starting from 0.")
    {
        this->seqno++;
        return true;
    }
};



#endif
