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

static const size_t GZIP_COMPRESSION_TRANSPORT_BUFFER_SIZE = 1024 * 64;

class GZipCompressionInTransport : public Transport {
    Transport & source_transport;

    z_stream compression_stream;

    char   * uncompressed_data;
    size_t   uncompressed_data_size;

    BStream uncompressed_buffer_stream;

    BStream compressed_data;

    bool inflate_pending;

    uint32_t verbose;

public:
    GZipCompressionInTransport(Transport & st, uint32_t verbose = 0)
    : Transport()
    , source_transport(st)
    , compression_stream()
    , uncompressed_data(NULL)
    , uncompressed_data_size(0)
    , uncompressed_buffer_stream(GZIP_COMPRESSION_TRANSPORT_BUFFER_SIZE)
    , compressed_data(GZIP_COMPRESSION_TRANSPORT_BUFFER_SIZE)
    , inflate_pending(false)
    , verbose(verbose) {
        int ret = ::inflateInit(&this->compression_stream);
(void)ret;
    }

    virtual ~GZipCompressionInTransport() {
        ::inflateEnd(&this->compression_stream);
    }

private:
    virtual void do_recv(char ** pbuffer, size_t len) {
        char * temp_buffer = *pbuffer;
        size_t temp_length = len;

        while (temp_length) {
            if (this->uncompressed_data_size) {
                REDASSERT(this->uncompressed_data);

                const size_t data_length = std::min<size_t>(temp_length, this->uncompressed_data_size);

                ::memcpy(temp_buffer, this->uncompressed_data, data_length);

                this->uncompressed_data      += data_length;
                this->uncompressed_data_size -= data_length;

                temp_buffer += data_length;
                temp_length -= data_length;
            }
            else {
                if (!this->inflate_pending) {
                    this->uncompressed_buffer_stream.reset();

                    this->source_transport.recv(
                          &this->uncompressed_buffer_stream.end
                        , 3                         // reset_decompressor((1) + compressed_data_size(2)
                        );

                    if (this->uncompressed_buffer_stream.in_uint8() == 1) {
                        if (this->verbose) {
                            LOG(LOG_INFO, "GZipCompressionInTransport::do_recv: Decompressor reset");
                        }
                        ::inflateEnd(&this->compression_stream);

                        ::memset(&this->compression_stream, 0, sizeof(this->compression_stream));

                        int ret = ::inflateInit(&this->compression_stream);
(void)ret;

                        this->inflate_pending = false;
                    }

                    uint16_t compressed_data_size = this->uncompressed_buffer_stream.in_uint16_le();
                    if (this->verbose) {
                        LOG(LOG_INFO, "GZipCompressionInTransport::do_recv: compressed_data_size=%u", compressed_data_size);
                    }

                    this->compressed_data.reset();

                    this->source_transport.recv(&this->compressed_data.end, compressed_data_size);

                    this->uncompressed_buffer_stream.reset();

                    this->compression_stream.avail_in = compressed_data_size;
                    this->compression_stream.next_in  = reinterpret_cast<unsigned char *>(this->compressed_data.get_data());
                }

                this->uncompressed_data = reinterpret_cast<char *>(this->uncompressed_buffer_stream.get_data());

                const size_t uncompressed_data_capacity = this->uncompressed_buffer_stream.get_capacity();

                this->compression_stream.avail_out = uncompressed_data_capacity;
                this->compression_stream.next_out  = reinterpret_cast<unsigned char *>(this->uncompressed_data);

                int ret = ::inflate(&this->compression_stream, Z_NO_FLUSH);
                if (this->verbose & 0x2) {
                    LOG(LOG_INFO, "GZipCompressionInTransport::do_recv: inflate return %d", ret);
                }
(void)ret;

                if (this->verbose & 0x2) {
                    LOG(LOG_INFO, "GZipCompressionInTransport::do_recv: uncompressed_data_capacity=%u avail_out=%u", uncompressed_data_capacity, this->compression_stream.avail_out);
                }
                this->uncompressed_data_size = uncompressed_data_capacity - this->compression_stream.avail_out;
                if (this->verbose) {
                    LOG(LOG_INFO, "GZipCompressionInTransport::do_recv: uncompressed_data_size=%u", this->uncompressed_data_size);
                }

                this->inflate_pending = ((ret == 0) && (this->compression_stream.avail_out == 0));
            }
        }

        (*pbuffer) = (*pbuffer) + len;
    }
};  // class GZipCompressionInTransport


class GZipCompressionOutTransport : public Transport {
    Transport & target_transport;

    z_stream compression_stream;

    bool reset_compressor;

    static const size_t MAX_DATA_SIZE = 1000 * 64; // ((GZIP_COMPRESSION_TRANSPORT_BUFFER_SIZE - 6) * 16384 - 5 * 16383) / (5 + 16384) = 65505

    char   uncompressed_data[GZIP_COMPRESSION_TRANSPORT_BUFFER_SIZE];
    size_t uncompressed_data_size;

    uint32_t verbose;

public:
    GZipCompressionOutTransport(Transport & tt, uint32_t verbose = 0)
    : Transport()
    , target_transport(tt)
    , compression_stream()
    , reset_compressor(false)
    , uncompressed_data()
    , uncompressed_data_size(0)
    , verbose(verbose) {
        REDASSERT(MAX_DATA_SIZE <= ((GZIP_COMPRESSION_TRANSPORT_BUFFER_SIZE - 6) * 16384 - 5 * 16383) / (5 + 16384));

        int ret = ::deflateInit(&this->compression_stream, Z_DEFAULT_COMPRESSION);
(void)ret;
    }

    virtual ~GZipCompressionOutTransport() {
        if (this->uncompressed_data_size) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "GZipCompressionOutTransport::~GZipCompressionOutTransport: Compress");
            }
            this->compress(this->uncompressed_data, this->uncompressed_data_size, true);

            this->uncompressed_data_size = 0;
        }

        ::deflateEnd(&this->compression_stream);
    }

private:
    void compress(const char * const buffer, size_t len, bool end) {
        if (this->verbose) {
            LOG(LOG_INFO, "GZipCompressionOutTransport::compress: uncompressed_data_size=%u", len);
        }
        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "GZipCompressionOutTransport::compress: end=%s", (end ? "true" : "false"));
        }
        const int flush = (end ? Z_FINISH : /*Z_NO_FLUSH*/Z_SYNC_FLUSH);

        this->compression_stream.avail_in = len;
        this->compression_stream.next_in  = reinterpret_cast<unsigned char *>(const_cast<char *>(buffer));

        char compressed_data[GZIP_COMPRESSION_TRANSPORT_BUFFER_SIZE];

        do {
            this->compression_stream.avail_out = sizeof(compressed_data);
            this->compression_stream.next_out  = reinterpret_cast<unsigned char *>(compressed_data);

            int ret = ::deflate(&this->compression_stream, flush);
(void)ret;
            if (this->verbose & 0x2) {
                LOG(LOG_INFO, "GZipCompressionOutTransport::compress: deflate return %d", ret);
            }
            REDASSERT(ret != Z_STREAM_ERROR);

            if (this->verbose & 0x2) {
                LOG(LOG_INFO, "GZipCompressionOutTransport::compress: compressed_data_capacity=%u avail_out=%u", sizeof(compressed_data), this->compression_stream.avail_out);
            }
            const uint16_t compressed_data_size = sizeof(compressed_data) - this->compression_stream.avail_out;

            BStream buffer_stream(128);

            buffer_stream.out_uint8(this->reset_compressor ? 1 : 0);
            this->reset_compressor = false;

            buffer_stream.out_uint16_le(compressed_data_size);
            if (this->verbose) {
                LOG(LOG_INFO, "GZipCompressionOutTransport::compress: compressed_data_size=%u", compressed_data_size);
            }

            buffer_stream.mark_end();
            this->target_transport.send(buffer_stream);

            this->target_transport.send(compressed_data, compressed_data_size);
        }
        while (this->compression_stream.avail_out == 0);
        REDASSERT(this->compression_stream.avail_in == 0);
    }

    virtual void do_send(const char * const buffer, size_t len) {
        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "GZipCompressionOutTransport::do_send: len=%u", len);
        }
        const char * temp_buffer = buffer;
        size_t       temp_length = len;

        while (temp_length) {
            if (this->uncompressed_data_size) {
                const size_t data_length = std::min<size_t>(
                      temp_length
                    , GZipCompressionOutTransport::MAX_DATA_SIZE - this->uncompressed_data_size
                    );

                ::memcpy(this->uncompressed_data + this->uncompressed_data_size, temp_buffer, data_length);

                this->uncompressed_data_size += data_length;

                temp_buffer += data_length;
                temp_length -= data_length;

                if (this->uncompressed_data_size == GZipCompressionOutTransport::MAX_DATA_SIZE) {
                    this->compress(this->uncompressed_data, this->uncompressed_data_size, false);

                    this->uncompressed_data_size = 0;
                }
            }
            else {
                if (temp_length > GZipCompressionOutTransport::MAX_DATA_SIZE) {
                    this->compress(temp_buffer, GZipCompressionOutTransport::MAX_DATA_SIZE, false);

                    temp_buffer += GZipCompressionOutTransport::MAX_DATA_SIZE;
                    temp_length -= GZipCompressionOutTransport::MAX_DATA_SIZE;
                }
                else {
                    ::memcpy(this->uncompressed_data, temp_buffer, temp_length);

                    this->uncompressed_data_size = temp_length;

                    temp_length = 0;
                }
            }
        }
        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "GZipCompressionOutTransport::do_send: uncompressed_data_size=%u", this->uncompressed_data_size);
        }
    }

public:
    virtual bool next() {
        if (this->uncompressed_data_size) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "GZipCompressionOutTransport::next: Compress");
            }
            this->compress(this->uncompressed_data, this->uncompressed_data_size, true);

            this->uncompressed_data_size = 0;
        }

        if (this->verbose) {
            LOG(LOG_INFO, "GZipCompressionOutTransport::next: Compressor reset");
        }

        ::deflateEnd(&this->compression_stream);

        ::memset(&this->compression_stream, 0, sizeof(this->compression_stream));

        int ret = ::deflateInit(&this->compression_stream, Z_DEFAULT_COMPRESSION);
(void)ret;

        this->reset_compressor = true;

        return this->target_transport.next();
    }

    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};  // class GZipCompressionOutTransport

#endif  // #ifndef REDEMPTION_TRANSPORT_GZIP_COMPRESSION_TRANSPORT_HPP
