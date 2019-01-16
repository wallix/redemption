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

#include <cinttypes>

#include <snappy-c.h>

#include "transport/transport.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"

static const size_t SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH = 1024 * 64;

/*******************************
* SnappyCompressionInTransport
*/

class SnappyCompressionInTransport : public Transport {
    Transport & source_transport;

    uint8_t * uncompressed_data;
    size_t    uncompressed_data_length = 0;
    uint8_t   uncompressed_data_buffer[SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

public:
    explicit SnappyCompressionInTransport(Transport & st, uint32_t verbose = 0)
    : source_transport(st)
    , uncompressed_data(nullptr)
    , uncompressed_data_buffer() {
        (void)verbose;
    }

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override {
        size_t    remaining_size = len;

        while (remaining_size) {
            if (this->uncompressed_data_length) {
                assert(this->uncompressed_data);

                const size_t data_length = std::min<size_t>(remaining_size, this->uncompressed_data_length);

                ::memcpy(&buffer[len-remaining_size], this->uncompressed_data, data_length);

                this->uncompressed_data        += data_length;
                this->uncompressed_data_length -= data_length;

                remaining_size -= data_length;
            }
            else {
                uint8_t data_buf[SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

                // read compressed_data_length(2);
                if (Read::Eof == this->source_transport.atomic_read(data_buf, sizeof(uint16_t))){
                    if (remaining_size == len) {
                        return Read::Eof;
                    }
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }

                const uint16_t compressed_data_length = Parse(data_buf).in_uint16_le();
                //if (this->verbose) {
                //    LOG(LOG_INFO, "SnappyCompressionInTransport::do_recv: compressed_data_length=%" PRIu16, compressed_data_length);
                //}
                this->source_transport.recv_boom(data_buf, compressed_data_length);
                this->uncompressed_data        = this->uncompressed_data_buffer;
                this->uncompressed_data_length = sizeof(this->uncompressed_data_buffer);

                snappy_status status = ::snappy_uncompress(
                      char_ptr_cast(data_buf) , compressed_data_length
                    , char_ptr_cast(this->uncompressed_data), &this->uncompressed_data_length);
                if (/*this->verbose & 0x2 || */(status != SNAPPY_OK)) {
                    LOG( ((status != SNAPPY_OK) ? LOG_ERR : LOG_INFO)
                       , "SnappyCompressionInTransport::do_recv: snappy_uncompress return %u", status);
                }
            }
        }
        return Read::Ok;
    }


};  // class SnappyCompressionInTransport


/********************************
* SnappyCompressionOutTransport
*/

class SnappyCompressionOutTransport : public Transport {
    Transport & target_transport;

    static const size_t MAX_UNCOMPRESSED_DATA_LENGTH = 56000; // ::snappy_max_compressed_length(56000) = 65365

    uint8_t uncompressed_data[SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  uncompressed_data_length = 0;

public:
    explicit SnappyCompressionOutTransport(Transport & tt, uint32_t verbose = 0)
    : target_transport(tt)
    , uncompressed_data()
    {
        (void)verbose;
        assert(::snappy_max_compressed_length(MAX_UNCOMPRESSED_DATA_LENGTH) <= SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH);
        static_assert(MAX_UNCOMPRESSED_DATA_LENGTH <= 0xFFFF); // 0xFFFF (for uint16_t)
    }

    ~SnappyCompressionOutTransport() override {
        if (this->uncompressed_data_length) {
            //if (this->verbose & 0x4) {
            //    LOG(LOG_INFO, "SnappyCompressionOutTransport::~SnappyCompressionOutTransport: Compress");
            //}
            this->compress(this->uncompressed_data, this->uncompressed_data_length);

            this->uncompressed_data_length = 0;
        }
    }

private:
    void compress(const uint8_t * const data, size_t data_length) const {
        //if (this->verbose) {
        //    LOG(LOG_INFO, "SnappyCompressionOutTransport::compress: data_length=%zu", data_length);
        //}

        StaticOutStream<SNAPPY_COMPRESSION_TRANSPORT_BUFFER_LENGTH> data_stream;
        size_t  compressed_data_length = data_stream.get_capacity();

        uint32_t compressed_data_length_offset = data_stream.get_offset();
        data_stream.out_skip_bytes(sizeof(uint16_t));
        compressed_data_length -= sizeof(uint16_t);

        snappy_status status = ::snappy_compress( char_ptr_cast(data), data_length
                                                , char_ptr_cast(data_stream.get_current()), &compressed_data_length);
        if (/*this->verbose & 0x2 || */(status != SNAPPY_OK)) {
            LOG( ((status != SNAPPY_OK) ? LOG_ERR : LOG_INFO)
               , "SnappyCompressionOutTransport::compress: snappy_compress return %u", status);
        }
        //if (this->verbose) {
        //    LOG(LOG_INFO, "SnappyCompressionOutTransport::compress: compressed_data_length=%zu", compressed_data_length);
        //}

        data_stream.out_skip_bytes(compressed_data_length);

        data_stream.set_out_uint16_le(compressed_data_length, compressed_data_length_offset);

        this->target_transport.send(data_stream.get_bytes());
    }

    void do_send(const uint8_t * const buffer, size_t len) override {
        //if (this->verbose & 0x4) {
        //    LOG(LOG_INFO, "SnappyCompressionOutTransport::do_send: len=%zu", len);
        //}

        const uint8_t * temp_data        = buffer;
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

        //if (this->verbose & 0x4) {
        //    LOG(LOG_INFO, "SnappyCompressionOutTransport::do_send: uncompressed_data_length=%zu", this->uncompressed_data_length);
        //}
    }

public:
    bool next() override {
        if (this->uncompressed_data_length) {
            //if (this->verbose & 0x4) {
            //    LOG(LOG_INFO, "SnappyCompressionOutTransport::next: Compress");
            //}
            this->compress(this->uncompressed_data, this->uncompressed_data_length);

            this->uncompressed_data_length = 0;
        }

        return this->target_transport.next();
    }

    void timestamp(timeval now) override {
        this->target_transport.timestamp(now);
    }
};  // class SnappyCompressionOutTransport

