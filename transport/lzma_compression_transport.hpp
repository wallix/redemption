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

#ifndef REDEMPTION_TRANSPORT_LZMA_COMPRESSION_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_LZMA_COMPRESSION_TRANSPORT_HPP

#include <lzma.h>

#include "transport.hpp"

static const size_t LZMA_COMPRESSION_TRANSPORT_BUFFER_LENGTH = 1024 * 64;

/******************************
* LzmaCompressionInTransport
*/

class LzmaCompressionInTransport : public Transport {
    Transport & source_transport;

    lzma_stream compression_stream;

    BStream compressed_data;

    uint8_t * uncompressed_data;
    size_t    uncompressed_data_length;
    uint8_t   uncompressed_data_buffer[LZMA_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

    bool decode_pending;

    uint32_t verbose;

public:
    LzmaCompressionInTransport(Transport & st, uint32_t verbose = 0)
    : Transport()
    , source_transport(st)
    , compression_stream()
    , compressed_data(LZMA_COMPRESSION_TRANSPORT_BUFFER_LENGTH)
    , uncompressed_data(NULL)
    , uncompressed_data_length(0)
    , uncompressed_data_buffer()
    , decode_pending(false)
    , verbose(verbose) {
        this->compression_stream = LZMA_STREAM_INIT;

        lzma_ret ret = ::lzma_stream_decoder( &this->compression_stream
                                            , UINT64_MAX                                        // No memory limit.
                                            , LZMA_TELL_UNSUPPORTED_CHECK | LZMA_CONCATENATED);
(void)ret;
    }

    virtual ~LzmaCompressionInTransport() {
        ::lzma_end(&this->compression_stream);
    }

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
                this->compressed_data.reset();

                if (!this->decode_pending) {
                    this->source_transport.recv(
                          &this->compressed_data.end
                        , 5                             // reset_decompressor(1) + compressed_data_length(4)
                        );

                    if (this->compressed_data.in_uint8() == 1) {
                        REDASSERT(this->decode_pending == false);

                        if (this->verbose) {
                            LOG(LOG_INFO, "LzmaCompressionInTransport::do_recv: Decompressor reset");
                        }
                        ::lzma_end(&this->compression_stream);

                        this->compression_stream = LZMA_STREAM_INIT;

                        lzma_ret ret = ::lzma_stream_decoder( &this->compression_stream
                                                            , UINT64_MAX                                        // No memory limit.
                                                            , LZMA_TELL_UNSUPPORTED_CHECK | LZMA_CONCATENATED);
(void)ret;
                    }

                    const size_t compressed_data_length = this->compressed_data.in_uint32_le();
                    if (this->verbose) {
                        LOG( LOG_INFO, "LzmaCompressionInTransport::do_recv: compressed_data_length=%u"
                           , compressed_data_length);
                    }

                    this->compressed_data.reset();

                    this->source_transport.recv(&this->compressed_data.end, compressed_data_length);

                    this->compression_stream.avail_in = compressed_data_length;
                    this->compression_stream.next_in  = this->compressed_data.get_data();
                }

                this->uncompressed_data = this->uncompressed_data_buffer;

                const size_t uncompressed_data_capacity = sizeof(this->uncompressed_data_buffer);

                this->compression_stream.avail_out = uncompressed_data_capacity;
                this->compression_stream.next_out  = this->uncompressed_data;

                lzma_ret ret = ::lzma_code(&this->compression_stream, LZMA_RUN);
                if (this->verbose & 0x2) {
                    LOG(LOG_INFO, "LzmaCompressionInTransport::do_recv: lzma_code return %d", ret);
                }
(void)ret;
                REDASSERT((ret == LZMA_OK) || (ret == LZMA_STREAM_END));

                if (this->verbose & 0x2) {
                    LOG( LOG_INFO
                       , "LzmaCompressionInTransport::do_recv: uncompressed_data_capacity=%u avail_out=%u"
                       , uncompressed_data_capacity, this->compression_stream.avail_out);
                }
                this->uncompressed_data_length = uncompressed_data_capacity - this->compression_stream.avail_out;
                if (this->verbose) {
                    LOG( LOG_INFO, "LzmaCompressionInTransport::do_recv: uncompressed_data_length=%u"
                       , this->uncompressed_data_length);
                }

                this->decode_pending =
                    ((ret == LZMA_OK) && (this->compression_stream.avail_out == 0));
            }
        }   // while (temp_data_length)

        (*pbuffer) = (*pbuffer) + len;
    }
};  // class LzmaCompressionInTransport


/******************************
* LzmaCompressionOutTransport
*/

class LzmaCompressionOutTransport : public Transport {
    Transport & target_transport;

    lzma_stream compression_stream;

    bool reset_compressor;

    uint8_t uncompressed_data[LZMA_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  uncompressed_data_length;

    uint8_t compressed_data[LZMA_COMPRESSION_TRANSPORT_BUFFER_LENGTH];
    size_t  compressed_data_length;

    bool improve_compression_ratio;

    uint32_t verbose;

public:
    LzmaCompressionOutTransport(Transport & tt, bool improve_compression_ratio = false, uint32_t verbose = 0)
    : Transport()
    , target_transport(tt)
    , compression_stream()
    , reset_compressor(false)
    , uncompressed_data()
    , uncompressed_data_length(0)
    , compressed_data()
    , compressed_data_length(0)
    , improve_compression_ratio(improve_compression_ratio)
    , verbose(verbose) {
        this->compression_stream = LZMA_STREAM_INIT;

        lzma_ret ret = ::lzma_easy_encoder( &this->compression_stream
                                          ,   6                                 // compression level
                                            | ( this->improve_compression_ratio
                                              ? LZMA_PRESET_EXTREME             // extreme compression
                                              : 0
                                              )
                                          , LZMA_CHECK_CRC64                    // integrity check type to use
                                          );
(void)ret;
    }

    virtual ~LzmaCompressionOutTransport() {
        if (this->uncompressed_data_length) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "LzmaCompressionOutTransport::~LzmaCompressionOutTransport: Compress");
            }
            this->compress(this->uncompressed_data, this->uncompressed_data_length, true);

            this->uncompressed_data_length = 0;
        }

        if (this->compressed_data_length) {
            this->send_to_target();
        }

        ::lzma_end(&this->compression_stream);
    }

private:
    void compress(const uint8_t * const data, size_t data_length, bool end) {
        if (this->verbose) {
            LOG(LOG_INFO, "LzmaCompressionOutTransport::compress: uncompressed_data_length=%u", data_length);
        }
        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "LzmaCompressionOutTransport::compress: end=%s", (end ? "true" : "false"));
        }

        const lzma_action action = (end ? LZMA_FINISH : LZMA_RUN);

        this->compression_stream.avail_in = data_length;
        this->compression_stream.next_in  = const_cast<uint8_t *>(data);

        uint8_t temp_compressed_data[LZMA_COMPRESSION_TRANSPORT_BUFFER_LENGTH];

        do {
            this->compression_stream.avail_out = sizeof(temp_compressed_data);
            this->compression_stream.next_out  = reinterpret_cast<unsigned char *>(temp_compressed_data);

            lzma_ret ret = ::lzma_code(&this->compression_stream, action);
(void)ret;
            if (this->verbose & 0x2) {
                LOG(LOG_INFO, "LzmaCompressionOutTransport::compress: lzma_code return %d", ret);
            }
            REDASSERT((ret == LZMA_OK) || (ret == LZMA_STREAM_END));

            if (this->verbose & 0x2) {
                LOG( LOG_INFO
                   , "LzmaCompressionOutTransport::compress: compressed_data_capacity=%u avail_out=%u"
                   , sizeof(temp_compressed_data), this->compression_stream.avail_out);
            }
            const size_t temp_compressed_data_length =
                sizeof(temp_compressed_data) - this->compression_stream.avail_out;
            if (this->verbose) {
                LOG( LOG_INFO
                   , "LzmaCompressionOutTransport::compress: temp_compressed_data_length=%u"
                   , temp_compressed_data_length);
            }

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
        REDASSERT(this->compression_stream.avail_in == 0);
    }

    virtual void do_send(const char * const buffer, size_t len) {
        if (this->verbose & 0x4) {
            LOG(LOG_INFO, "LzmaCompressionOutTransport::do_send: len=%u", len);
        }

        const uint8_t * temp_data        = reinterpret_cast<const uint8_t *>(buffer);
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

        if (this->verbose & 0x4) {
            LOG( LOG_INFO, "LzmaCompressionOutTransport::do_send: uncompressed_data_length=%u"
               , this->uncompressed_data_length);
        }
    }

public:
    virtual bool next() {
        if (this->uncompressed_data_length) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "LzmaCompressionOutTransport::next: Compress");
            }
            this->compress(this->uncompressed_data, this->uncompressed_data_length, true);

            this->uncompressed_data_length = 0;
        }

        if (this->compressed_data_length) {
            this->send_to_target();
        }

        if (this->verbose) {
            LOG(LOG_INFO, "LzmaCompressionOutTransport::next: Compressor reset");
        }

        ::lzma_end(&this->compression_stream);

        this->compression_stream = LZMA_STREAM_INIT;

        lzma_ret ret = ::lzma_easy_encoder( &this->compression_stream
                                          ,   6                                 // compression level
                                            | ( this->improve_compression_ratio
                                              ? LZMA_PRESET_EXTREME             // extreme compression
                                              : 0
                                              )
                                          , LZMA_CHECK_CRC64                    // integrity check type to use
                                          );
(void)ret;

        this->reset_compressor = true;

        return this->target_transport.next();
    }

private:
    void send_to_target() {
        if (!this->compressed_data_length)
            return;

        BStream buffer_stream(128);

        buffer_stream.out_uint8(this->reset_compressor ? 1 : 0);
        this->reset_compressor = false;

        buffer_stream.out_uint32_le(this->compressed_data_length);
        if (this->verbose) {
            LOG( LOG_INFO
               , "LzmaCompressionOutTransport::send_to_target: compressed_data_length=%u"
               , this->compressed_data_length);
        }

        buffer_stream.mark_end();
        this->target_transport.send(buffer_stream);

        this->target_transport.send(this->compressed_data, this->compressed_data_length);

        this->compressed_data_length = 0;
    }

public:
    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};  // class LzmaCompressionOutTransport

#endif  // #ifndef REDEMPTION_TRANSPORT_LZMA_COMPRESSION_TRANSPORT_HPP
