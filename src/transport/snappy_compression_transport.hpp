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


static const size_t SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH = 1024 * 64;

class SnappyCompressionInTransport : public Transport
{
    Transport & source_transport;

    uint8_t * uncompressed_data = nullptr;
    size_t    uncompressed_data_length = 0;
    uint8_t   uncompressed_data_buffer[SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH] {};

public:
    explicit SnappyCompressionInTransport(Transport & st)
    : source_transport(st)
    {}

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override;
};


class SnappyCompressionOutTransport : public Transport
{
    Transport & target_transport;

    static const size_t MAX_UNCOMPRESSED_DATA_LENGTH = 56000; // ::snappy_max_compressed_length(56000) = 65365

    uint8_t uncompressed_data[SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH] {};
    size_t  uncompressed_data_length = 0;

public:
    explicit SnappyCompressionOutTransport(Transport & tt);

    ~SnappyCompressionOutTransport() override;

private:
    void compress(const uint8_t * const data, size_t data_length) const;

    void do_send(const uint8_t * const buffer, size_t len) override;

public:
    bool next() override;
};
