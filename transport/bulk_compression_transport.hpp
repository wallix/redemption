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

#ifndef REDEMPTION_TRANSPORT_BULK_COMPRESSION_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_BULK_COMPRESSION_TRANSPORT_HPP

#include "transport.hpp"
#include "RDP/mppc.hpp"
#include "RDP/mppc_61.hpp"

class BulkCompressionInTransport : public Transport {
    Transport & source_transport;

    rdp_mppc_61_dec * mppc_dec;

    const uint8_t * uncompressed_data;
    uint32_t        uncompressed_data_size;

    BStream buffer_stream;

public:
    BulkCompressionInTransport(Transport & st)
    : Transport()
    , source_transport(st)
    , mppc_dec(NULL)
    , uncompressed_data(NULL)
    , uncompressed_data_size(0)
    , buffer_stream(65536) {
        this->mppc_dec = new rdp_mppc_61_dec();
    }

    virtual ~BulkCompressionInTransport() {
        delete this->mppc_dec;
    }

private:
    virtual void do_recv(char ** pbuffer, size_t len) {
        char * temp_buffer = *pbuffer;
        size_t temp_length = len;

        while (temp_length) {
            if (uncompressed_data_size) {
                REDASSERT(uncompressed_data);

                const size_t data_length = std::min<size_t>(temp_length, uncompressed_data_size);

                ::memcpy(temp_buffer, uncompressed_data, data_length);

                uncompressed_data      += data_length;
                uncompressed_data_size -= data_length;

                temp_buffer += data_length;
                temp_length -= data_length;
            }
            else {
                buffer_stream.reset();

                this->source_transport.recv(
                      &buffer_stream.end
                    , 4                     // reset_compressor((1) + compressed_type(1) + data_size(2)
                    );

                if (buffer_stream.in_uint8() == 1) {
                    delete this->mppc_dec;

                    this->mppc_dec = new rdp_mppc_61_dec();
                }

                uint8_t  compressed_type = buffer_stream.in_uint8();
                uint16_t data_size       = buffer_stream.in_uint16_le();
                //LOG(LOG_INFO, "do_recv: data_size=%u", data_size);

                buffer_stream.reset();

                this->source_transport.recv(&buffer_stream.end, data_size);

                if (compressed_type & PACKET_COMPRESSED) {
                    this->mppc_dec->decompress(buffer_stream.get_data(), data_size,
                        compressed_type, uncompressed_data, uncompressed_data_size);
                }
                else {
                    uncompressed_data      = buffer_stream.get_data();
                    uncompressed_data_size = data_size;
                }
            }
        }

        (*pbuffer) = (*pbuffer) + len;
    }
};  // class BulkCompressionInTransport


class BulkCompressionOutTransport : public Transport {
    Transport & target_transport;

    rdp_mppc_61_enc_hash_based_match_finder * mppc_enc_match_finder;
    rdp_mppc_61_enc                         * mppc_enc;

    bool reset_compressor;

    uint32_t verbose_compression;

public:
    BulkCompressionOutTransport(Transport & tt, uint32_t verbose_compression = 0)
    : Transport()
    , target_transport(tt)
    , mppc_enc_match_finder(NULL)
    , mppc_enc(NULL)
    , reset_compressor(false)
    , verbose_compression(verbose_compression) {
        this->mppc_enc_match_finder = new rdp_mppc_61_enc_hash_based_match_finder();
        this->mppc_enc              = new rdp_mppc_61_enc(this->mppc_enc_match_finder, verbose_compression);
    }

    virtual ~BulkCompressionOutTransport () {
        delete this->mppc_enc_match_finder;
        delete this->mppc_enc;
    }

private:
    virtual void do_send(const char * const buffer, size_t len) {
        const char * temp_buffer = buffer;
        size_t       temp_length = len;

        const size_t MAX_DATA_LENGTH = 1024 * 60;

        BStream compressed_buffer_stream(65536);

        while (temp_length) {
            const size_t data_length = (temp_length > MAX_DATA_LENGTH) ? MAX_DATA_LENGTH : temp_length;

            uint8_t  compressed_type      = 0;
            uint16_t compressed_data_size = 0;

            this->mppc_enc->compress(reinterpret_cast<const uint8_t *>(temp_buffer), data_length,
                compressed_type, compressed_data_size, rdp_mppc_enc::MAX_COMPRESSED_DATA_SIZE_UNUSED);

            compressed_buffer_stream.reset();

            compressed_buffer_stream.out_uint8(reset_compressor ? 1 : 0);
            compressed_buffer_stream.out_uint8(compressed_type);

            if (compressed_type & PACKET_COMPRESSED) {
                compressed_buffer_stream.out_uint16_le(compressed_data_size);

                this->mppc_enc->get_compressed_data(compressed_buffer_stream);

                compressed_buffer_stream.mark_end();

                this->target_transport.send(compressed_buffer_stream);
            }
            else {
                compressed_buffer_stream.out_uint16_le(data_length);

                compressed_buffer_stream.mark_end();

                this->target_transport.send(compressed_buffer_stream);

                this->target_transport.send(temp_buffer, data_length);
            }

            temp_buffer += data_length;
            temp_length -= data_length;
        }
    }

public:
    virtual bool next() {
        this->reset_compressor = true;

        delete this->mppc_enc_match_finder;
        delete this->mppc_enc;

        this->mppc_enc_match_finder = new rdp_mppc_61_enc_hash_based_match_finder();
        this->mppc_enc              = new rdp_mppc_61_enc(this->mppc_enc_match_finder, this->verbose_compression);

        return this->target_transport.next();
    }

    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};  // class BulkCompressionOutTransport

#endif  // #ifndef REDEMPTION_TRANSPORT_BULK_COMPRESSION_TRANSPORT_HPP
