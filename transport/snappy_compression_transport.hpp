/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#include "transport.hpp"

#ifndef REDEMPTION_TRANSPORT_SNAPPY_COMPRESSION_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_SNAPPY_COMPRESSION_TRANSPORT_HPP

static const size_t SNAPPY_COMPRESSION_TRANSPORT_BUFFER_SIZE = 1024 * 64;

class SnappyCompressionInTransport : public Transport {
    Transport & source_transport;

    uint32_t verbose;

public:
    SnappyCompressionInTransport(Transport & st, uint32_t verbose = 0)
    : Transport()
    , source_transport(st)
    , verbose(verbose) {
    }

private:
    virtual void do_recv(char ** pbuffer, size_t len) {
    }
};  // class SnappyCompressionInTransport


class GZipCompressionOutTransport : public Transport {
    Transport & target_transport;

    uint32_t verbose;

public:
    GZipCompressionOutTransport(Transport & tt, uint32_t verbose = 0)
    : Transport()
    , target_transport(tt)
    , verbose(verbose) {
    }

public:
    virtual bool next() {
        return this->target_transport.next();
    }

    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};

#endif  // #ifndef REDEMPTION_TRANSPORT_SNAPPY_COMPRESSION_TRANSPORT_HPP
