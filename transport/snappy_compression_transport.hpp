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

#ifndef REDEMPTION_TRANSPORT_SNAPPY_COMPRESSION_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_SNAPPY_COMPRESSION_TRANSPORT_HPP

#include <snappy-c.h>

#include "transport.hpp"

static const size_t SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH = 1024 * 64;

/*******************************
* SnappyCompressionInTransport
*/

class SnappyCompressionInTransport : public Transport {
    Transport & source_transport;

    uint8_t * uncompressed_data;
    size_t    uncompressed_data_length;
    uint8_t   uncompressed_data_buffer[SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

    uint32_t verbose;

public:
    SnappyCompressionInTransport(Transport & st, uint32_t verbose = 0)
    : Transport()
    , source_transport(st)
    , uncompressed_data(NULL)
    , uncompressed_data_length(0)
    , uncompressed_data_buffer()
    , verbose(verbose) {}

private:
    virtual void do_recv(char ** pbuffer, size_t len) {
        uint8_t * temp_data        = reinterpret_cast<uint8_t *>(*pbuffer);
        size_t    temp_data_length = len;

        while (temp_data_length) {
            if (this->uncompressed_data_length) {
                REDASSERT(this->uncompressed_data);

                const size_t data_length = std::min<size_t>(temp_data_length, this->uncompressed_data_length);

                ::memcpy(temp_data, this->uncompressed_data, data_length);

                this->uncompressed_data        += data_length;
                this->uncompressed_data_length -= data_length;

                temp_data        += data_length;
                temp_data_length -= data_length;
            }
            else {
                BStream data_stream(SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH);

                this->source_transport.recv(&data_stream.end, sizeof(uint16_t));  // compressed_data_length(2);

                const uint16_t compressed_data_length = data_stream.in_uint16_le();
                if (this->verbose) {
                    LOG(LOG_INFO, "SnappyCompressionInTransport::do_recv: compressed_data_length=%u", compressed_data_length);
                }

                data_stream.reset();

                this->source_transport.recv(&data_stream.end, compressed_data_length);

                this->uncompressed_data        = this->uncompressed_data_buffer;
                this->uncompressed_data_length = sizeof(this->uncompressed_data_buffer);

                snappy_status status = ::snappy_uncompress(
                      reinterpret_cast<char *>(data_stream.get_data()) , data_stream.size()
                    , reinterpret_cast<char *>(this->uncompressed_data), &this->uncompressed_data_length);
                if (this->verbose & 0x2 || (status != SNAPPY_OK)) {
                    LOG( ((status != SNAPPY_OK) ? LOG_ERR : LOG_INFO)
                       , "SnappyCompressionInTransport::do_recv: snappy_uncompress return %d", status);
                }
                if (this->verbose) {
                    LOG( LOG_INFO, "SnappyCompressionInTransport::do_recv: uncompressed_data_length=%u"
                       , this->uncompressed_data_length);
                }
            }
        }

        (*pbuffer) = (*pbuffer) + len;
    }
};  // class SnappyCompressionInTransport


/********************************
* SnappyCompressionOutTransport
*/

class SnappyCompressionOutTransport : public Transport {
    Transport & target_transport;

    static const size_t MAX_UNCOMPRESSED_DATA_LENGTH = 56000; // ::snappy_max_compressed_length(56000) = 65365

    uint8_t uncompressed_data[SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  uncompressed_data_length;

    uint32_t verbose;

public:
    SnappyCompressionOutTransport(Transport & tt, uint32_t verbose = 0)
    : Transport()
    , target_transport(tt)
    , uncompressed_data()
    , uncompressed_data_length(0)
    , verbose(verbose) {
        REDASSERT(::snappy_max_compressed_length(MAX_UNCOMPRESSED_DATA_LENGTH) <= SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH);
        REDASSERT(MAX_UNCOMPRESSED_DATA_LENGTH <= 0xFFFF); // 0xFFFF (for uint16_t)
    }

    virtual ~SnappyCompressionOutTransport() {
        if (this->uncompressed_data_length) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "SnappyCompressionOutTransport::~SnappyCompressionOutTransport: Compress");
            }
            this->compress(this->uncompressed_data, this->uncompressed_data_length);

            this->uncompressed_data_length = 0;
        }
    }

private:
    void compress(const uint8_t * const data, size_t data_length) const {
        if (this->verbose) {
            LOG(LOG_INFO, "SnappyCompressionOutTransport::compress: data_length=%u", data_length);
        }

        BStream data_stream(SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH);
        size_t  compressed_data_length = data_stream.get_capacity();

        uint32_t compressed_data_length_offset = data_stream.get_offset();
        data_stream.out_skip_bytes(sizeof(uint16_t));
        data_stream.mark_end();
        compressed_data_length -= sizeof(uint16_t);

        snappy_status status = ::snappy_compress( reinterpret_cast<const char *>(data), data_length
                                                , reinterpret_cast<char *>(data_stream.end), &compressed_data_length);
        if (this->verbose & 0x2 || (status != SNAPPY_OK)) {
            LOG( ((status != SNAPPY_OK) ? LOG_ERR : LOG_INFO)
               , "SnappyCompressionOutTransport::compress: snappy_compress return %d", status);
        }
        if (this->verbose) {
            LOG(LOG_INFO, "SnappyCompressionOutTransport::compress: compressed_data_length=%u", compressed_data_length);
        }

        data_stream.out_skip_bytes(compressed_data_length);
        data_stream.mark_end();

        data_stream.set_out_uint16_le(compressed_data_length, compressed_data_length_offset);

        this->target_transport.send(data_stream);
    }

    virtual void do_send(const char * const buffer, size_t len) {
        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "SnappyCompressionOutTransport::do_send: len=%u", len);
        }

        const uint8_t * temp_data        = reinterpret_cast<const uint8_t *>(buffer);
        size_t          temp_data_length = len;

        while (temp_data_length) {
            if (this->uncompressed_data_length) {
                const size_t data_length = std::min<size_t>(
                      temp_data_length
                    , MAX_UNCOMPRESSED_DATA_LENGTH - this->uncompressed_data_length);

                ::memcpy(this->uncompressed_data + this->uncompressed_data_length, temp_data, data_length);

                this->uncompressed_data_length += data_length;

                temp_data        += data_length;
                temp_data_length -= data_length;

                if (this->uncompressed_data_length == MAX_UNCOMPRESSED_DATA_LENGTH) {
                    this->compress(this->uncompressed_data, this->uncompressed_data_length);

                    this->uncompressed_data_length = 0;
                }
            }
            else {
                if (temp_data_length >= MAX_UNCOMPRESSED_DATA_LENGTH) {
                    this->compress(temp_data, MAX_UNCOMPRESSED_DATA_LENGTH);

                    temp_data        += MAX_UNCOMPRESSED_DATA_LENGTH;
                    temp_data_length -= MAX_UNCOMPRESSED_DATA_LENGTH;
                }
                else {
                    ::memcpy(this->uncompressed_data, temp_data, temp_data_length);

                    this->uncompressed_data_length = temp_data_length;

                    temp_data_length = 0;
                }
            }
        }

        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "SnappyCompressionOutTransport::do_send: uncompressed_data_length=%u", this->uncompressed_data_length);
        }
    }

public:
    virtual bool next() {
        if (this->uncompressed_data_length) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "SnappyCompressionOutTransport::next: Compress");
            }
            this->compress(this->uncompressed_data, this->uncompressed_data_length);

            this->uncompressed_data_length = 0;
        }

        return this->target_transport.next();
    }

    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};  // class SnappyCompressionOutTransport

#endif  // #ifndef REDEMPTION_TRANSPORT_SNAPPY_COMPRESSION_TRANSPORT_HPP
