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

#include <zlib.h>

#include "transport.hpp"

#ifndef REDEMPTION_TRANSPORT_GZIP_COMPRESSION_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_GZIP_COMPRESSION_TRANSPORT_HPP

class GZipCompressionInTransport : public Transport {
    Transport & source_transport;

    z_stream compression_stream;

public:
    GZipCompressionInTransport(Transport & st)
    : Transport()
    , source_transport(st)
    , compression_stream() {
        int ret = ::inflateInit(&compression_stream);
(void)ret;
    }

private:
    virtual void do_recv(char ** pbuffer, size_t len) {
    }
};  // class GZipCompressionInTransport


class GZipCompressionOutTransport : public Transport {
    Transport & target_transport;

    z_stream compression_stream;

public:
    GZipCompressionOutTransport(Transport & tt)
    : Transport()
    , target_transport(tt)
    , compression_stream() {
        int ret = ::deflateInit(&compression_stream, Z_DEFAULT_COMPRESSION);
(void)ret;
    }

private:
    virtual void do_send(const char * const buffer, size_t len) {
    }

public:
    virtual bool next() {
        return this->target_transport.next();
    }

    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};  // class GZipCompressionOutTransport

#endif  // #ifndef REDEMPTION_TRANSPORT_GZIP_COMPRESSION_TRANSPORT_HPP
