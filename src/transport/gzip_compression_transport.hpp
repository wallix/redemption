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

#pragma once

#include "transport/transport.hpp"
#include <zlib.h>


constexpr size_t GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH = 1024 * 64;


class GZipCompressionInTransport : public Transport
{
    Transport & source_transport;

    z_stream compression_stream;

    uint8_t compressed_data_buf[GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

    uint8_t * uncompressed_data;
    size_t    uncompressed_data_length;
    uint8_t   uncompressed_data_buffer[GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

    bool inflate_pending;

public:
    explicit GZipCompressionInTransport(Transport & st);

    ~GZipCompressionInTransport() override;

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override;
};


class GZipCompressionOutTransport : public Transport
{
    Transport & target_transport;

    z_stream compression_stream;

    bool reset_compressor;

    uint8_t uncompressed_data[GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  uncompressed_data_length = 0;

    uint8_t compressed_data[GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  compressed_data_length = 0;

public:
    explicit GZipCompressionOutTransport(Transport & tt);

    ~GZipCompressionOutTransport() override;

    bool next() override;

    void timestamp(timeval now) override {
        this->target_transport.timestamp(now);
    }

private:
    void compress(const uint8_t * const data, size_t data_length, bool end);

    void do_send(const uint8_t * const buffer, size_t len) override;

    void send_to_target();
};  // class GZipCompressionOutTransport
