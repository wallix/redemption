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
#include "utils/stream.hpp"
#include "cxx/diagnostic.hpp"

#include <cassert>
#include <zlib.h>

constexpr size_t GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH = 1024 * 64;

/*****************************
* GZipCompressionInTransport
*/

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
    explicit GZipCompressionInTransport(Transport & st, uint32_t verbose = 0)
    : source_transport(st)
    , compression_stream()
    , uncompressed_data(nullptr)
    , uncompressed_data_length(0)
    , uncompressed_data_buffer()
    , inflate_pending(false)
    //, verbose(verbose)
    {
        (void)verbose;
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
        int const ret = ::inflateInit(&this->compression_stream);
        REDEMPTION_DIAGNOSTIC_POP
        if (ret != Z_OK) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }

    ~GZipCompressionInTransport() override {
        ::inflateEnd(&this->compression_stream);
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
                if (!this->inflate_pending) {
                    // reset_decompressor(1) + compressed_data_length(4)
                    if (Read::Eof == this->source_transport.atomic_read(this->compressed_data_buf, 5)){
                        return Read::Eof;
                    }

                    InStream compressed_data(this->compressed_data_buf);

                    if (compressed_data.in_uint8() == 1) {
                        assert(this->inflate_pending == false);

                        ::inflateEnd(&this->compression_stream);

                        ::memset(&this->compression_stream, 0, sizeof(this->compression_stream));

                        REDEMPTION_DIAGNOSTIC_PUSH
                        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
                        int ret = ::inflateInit(&this->compression_stream);
                        REDEMPTION_DIAGNOSTIC_POP
                        (void)ret;
                    }

                    const size_t compressed_data_length = compressed_data.in_uint32_le();
                    this->source_transport.recv_boom(this->compressed_data_buf, compressed_data_length);
                    this->compression_stream.avail_in = compressed_data_length;
                    this->compression_stream.next_in  = this->compressed_data_buf;
                }

                this->uncompressed_data = this->uncompressed_data_buffer;

                const size_t uncompressed_data_capacity = sizeof(this->uncompressed_data_buffer);

                this->compression_stream.avail_out = uncompressed_data_capacity;
                this->compression_stream.next_out  = this->uncompressed_data;

                // TODO: what happens if some decompression error occurs ?
                int ret = ::inflate(&this->compression_stream, Z_NO_FLUSH);
                this->uncompressed_data_length = uncompressed_data_capacity - this->compression_stream.avail_out;

                this->inflate_pending = ((ret == 0) && (this->compression_stream.avail_out == 0));
            }
        }
        return Read::Ok;
    }


};  // class GZipCompressionInTransport


/******************************
* GZipCompressionOutTransport
*/

class GZipCompressionOutTransport : public Transport {
    Transport & target_transport;

    z_stream compression_stream;

    bool reset_compressor;

    uint8_t uncompressed_data[GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  uncompressed_data_length = 0;

    uint8_t compressed_data[GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  compressed_data_length = 0;

public:
    explicit GZipCompressionOutTransport(Transport & tt, uint32_t verbose = 0)
    : target_transport(tt)
    , compression_stream()
    , reset_compressor(false)
    , uncompressed_data()
    , compressed_data()
    {
        (void)verbose;
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
        int const ret = ::deflateInit(&this->compression_stream, Z_DEFAULT_COMPRESSION);
        REDEMPTION_DIAGNOSTIC_POP
        if (ret != Z_OK) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }

    ~GZipCompressionOutTransport() override {
        if (this->uncompressed_data_length) {
            //if (this->verbose & 0x4) {
            //    LOG(LOG_INFO, "GZipCompressionOutTransport::~GZipCompressionOutTransport: Compress");
            //}
            this->compress(this->uncompressed_data, this->uncompressed_data_length, true);

            this->uncompressed_data_length = 0;
        }

        if (this->compressed_data_length) {
            this->send_to_target();
        }

        ::deflateEnd(&this->compression_stream);
    }

private:
    void compress(const uint8_t * const data, size_t data_length, bool end) {
        //if (this->verbose) {
        //    LOG(LOG_INFO, "GZipCompressionOutTransport::compress: uncompressed_data_length=%zu", data_length);
        //}
        //if (this->verbose & 0x4) {
        //    LOG(LOG_INFO, "GZipCompressionOutTransport::compress: end=%s", (end ? "true" : "false"));
        //}

        const int flush = (end ? Z_FINISH : Z_NO_FLUSH);

        this->compression_stream.avail_in = data_length;
        this->compression_stream.next_in  = const_cast<uint8_t *>(data); /*NOLINT*/

        uint8_t temp_compressed_data[GZIP_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

        do {
            this->compression_stream.avail_out = sizeof(temp_compressed_data);
            this->compression_stream.next_out  = temp_compressed_data;

            int ret = ::deflate(&this->compression_stream, flush);
            //if (this->verbose & 0x2) {
            //    LOG(LOG_INFO, "GZipCompressionOutTransport::compress: deflate return %d", ret);
            //}
            (void)ret;
            assert(ret != Z_STREAM_ERROR);

            //if (this->verbose & 0x2) {
            //    LOG( LOG_INFO
            //       , "GZipCompressionOutTransport::compress: compressed_data_capacity=%zu avail_out=%u"
            //       , sizeof(temp_compressed_data), this->compression_stream.avail_out);
            //}
            const size_t temp_compressed_data_length =
                sizeof(temp_compressed_data) - this->compression_stream.avail_out;
            //if (this->verbose) {
            //    LOG( LOG_INFO
            //       , "GZipCompressionOutTransport::compress: temp_compressed_data_length=%zu"
            //       , temp_compressed_data_length);
            //}

            for (size_t number_of_bytes_copied = 0; number_of_bytes_copied < temp_compressed_data_length; ) {
                const size_t number_of_bytes_to_copy =
                    std::min<size_t>( sizeof(this->compressed_data) - this->compressed_data_length
                                    , temp_compressed_data_length - number_of_bytes_copied);

                ::memcpy( this->compressed_data + this->compressed_data_length
                        , temp_compressed_data + number_of_bytes_copied
                        , number_of_bytes_to_copy);

                this->compressed_data_length += number_of_bytes_to_copy;
                number_of_bytes_copied       += number_of_bytes_to_copy;

                if (this->compressed_data_length == sizeof(this->compressed_data)) {
                    this->send_to_target();
                }
            }
        }
        while (this->compression_stream.avail_out == 0);
        assert(this->compression_stream.avail_in == 0);
    }

    void do_send(const uint8_t * const buffer, size_t len) override {
        //if (this->verbose & 0x4) {
        //    LOG(LOG_INFO, "GZipCompressionOutTransport::do_send: len=%zu", len);
        //}

        const uint8_t * temp_data        = buffer;
        size_t          temp_data_length = len;

        while (temp_data_length) {
            if (this->uncompressed_data_length) {
                const size_t data_length = std::min<size_t>(
                      temp_data_length
                    , sizeof(this->uncompressed_data) - this->uncompressed_data_length
                    );

                ::memcpy(this->uncompressed_data + this->uncompressed_data_length, temp_data, data_length);

                this->uncompressed_data_length += data_length;

                temp_data += data_length;
                temp_data_length -= data_length;

                if (this->uncompressed_data_length == sizeof(this->uncompressed_data)) {
                    this->compress(this->uncompressed_data, this->uncompressed_data_length, false);

                    this->uncompressed_data_length = 0;
                }
            }
            else {
                if (temp_data_length >= sizeof(this->uncompressed_data)) {
                    this->compress(temp_data, sizeof(this->uncompressed_data), false);

                    temp_data += sizeof(this->uncompressed_data);
                    temp_data_length -= sizeof(this->uncompressed_data);
                }
                else {
                    ::memcpy(this->uncompressed_data, temp_data, temp_data_length);

                    this->uncompressed_data_length = temp_data_length;

                    temp_data_length = 0;
                }
            }
        }

        //if (this->verbose & 0x4) {
        //    LOG( LOG_INFO, "GZipCompressionOutTransport::do_send: uncompressed_data_length=%zu"
        //       , this->uncompressed_data_length);
        //}
    }

public:
    bool next() override {
        if (this->uncompressed_data_length) {
            //if (this->verbose & 0x4) {
            //    LOG(LOG_INFO, "GZipCompressionOutTransport::next: Compress");
            //}
            this->compress(this->uncompressed_data, this->uncompressed_data_length, true);

            this->uncompressed_data_length = 0;
        }

        if (this->compressed_data_length) {
            this->send_to_target();
        }

        //if (this->verbose) {
        //    LOG(LOG_INFO, "GZipCompressionOutTransport::next: Compressor reset");
        //}

        ::deflateEnd(&this->compression_stream);

        ::memset(&this->compression_stream, 0, sizeof(this->compression_stream));

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
        int ret = ::deflateInit(&this->compression_stream, Z_DEFAULT_COMPRESSION);
        (void)ret;
        REDEMPTION_DIAGNOSTIC_POP

        this->reset_compressor = true;

        return this->target_transport.next();
    }

private:
    void send_to_target() {
        if (!this->compressed_data_length) {
            return;
        }

        StaticOutStream<128> buffer_stream;

        buffer_stream.out_uint8(this->reset_compressor ? 1 : 0);
        this->reset_compressor = false;

        buffer_stream.out_uint32_le(this->compressed_data_length);
        //if (this->verbose) {
        //    LOG( LOG_INFO
        //       , "GZipCompressionOutTransport::send_to_target: compressed_data_length=%zu"
        //       , this->compressed_data_length);
        //}

        this->target_transport.send(buffer_stream.get_bytes());

        this->target_transport.send(this->compressed_data, this->compressed_data_length);

        this->compressed_data_length = 0;
    }

public:
    void timestamp(timeval now) override {
        this->target_transport.timestamp(now);
    }
};  // class GZipCompressionOutTransport
