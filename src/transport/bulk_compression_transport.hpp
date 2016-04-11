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

#include "transport/transport.hpp"
#include "core/RDP/mppc_61.hpp"

/*****************************
* BulkCompressionInTransport
*/

class BulkCompressionInTransport : public Transport {
    Transport & source_transport;

    rdp_mppc_61_dec mppc_dec;

    const uint8_t * uncompressed_data;
    uint32_t        uncompressed_data_size;

    uint8_t compressed_buffer[65536];
    uint8_t uncompressed_buffer[65536];

public:
    BulkCompressionInTransport(Transport & st)
    : Transport()
    , source_transport(st)
    , uncompressed_data(nullptr)
    , uncompressed_data_size(0) {}

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
                auto end = this->uncompressed_buffer;
                this->source_transport.recv(
                      &end
                    , 4                         // reset_compressor((1) + compressed_type(1) + data_size(2)
                    );

                InStream uncompressed_buffer_stream(this->uncompressed_buffer);

                if (uncompressed_buffer_stream.in_uint8() == 1) {
                    this->mppc_dec.~rdp_mppc_61_dec();

                    new (&this->mppc_dec) rdp_mppc_61_dec;
                }

                uint8_t  compressed_type = uncompressed_buffer_stream.in_uint8();
                uint16_t data_size       = uncompressed_buffer_stream.in_uint16_le();
                //LOG(LOG_INFO, "do_recv: data_size=%u", data_size);

                end = this->compressed_buffer;
                this->source_transport.recv(&end, data_size);

                if (compressed_type & PACKET_COMPRESSED) {
                    this->uncompressed_data = this->uncompressed_buffer;

                    this->mppc_dec.decompress(compressed_buffer, data_size,
                        compressed_type, this->uncompressed_data, this->uncompressed_data_size);
                }
                else {
                    this->uncompressed_data      = this->compressed_buffer;
                    this->uncompressed_data_size = data_size;
                }
            }
        }

        (*pbuffer) = (*pbuffer) + len;
    }
};  // class BulkCompressionInTransport


/******************************
* BulkCompressionOutTransport
*/

class BulkCompressionOutTransport : public Transport {
    Transport & target_transport;

    rdp_mppc_61_enc_hash_based mppc_enc;

    bool reset_compressor;

    uint32_t verbose_compression;

public:
    BulkCompressionOutTransport(Transport & tt, uint32_t verbose_compression = 0)
    : target_transport(tt)
    , mppc_enc(verbose_compression)
    , reset_compressor(false)
    , verbose_compression(verbose_compression) {}

private:
    virtual void do_send(const char * const buffer, size_t len) {
        const char * temp_buffer = buffer;
        size_t       temp_length = len;

        const size_t MAX_DATA_LENGTH = 1024 * 60;

        StaticOutStream<65536> buffer_stream;

        while (temp_length) {
            const size_t data_length = (temp_length > MAX_DATA_LENGTH) ? MAX_DATA_LENGTH : temp_length;

            uint8_t  compressed_type      = 0;
            uint16_t compressed_data_size = 0;

            this->mppc_enc.compress(reinterpret_cast<const uint8_t *>(temp_buffer), data_length,
                compressed_type, compressed_data_size, rdp_mppc_enc::MAX_COMPRESSED_DATA_SIZE_UNUSED);

            buffer_stream.rewind();

            buffer_stream.out_uint8(this->reset_compressor ? 1 : 0);
            this->reset_compressor = false;

            buffer_stream.out_uint8(compressed_type);

            if (compressed_type & PACKET_COMPRESSED) {
                buffer_stream.out_uint16_le(compressed_data_size);

                this->mppc_enc.get_compressed_data(buffer_stream);

                this->target_transport.send(buffer_stream.get_data(), buffer_stream.get_offset());
            }
            else {
                buffer_stream.out_uint16_le(data_length);

                this->target_transport.send(buffer_stream.get_data(), buffer_stream.get_offset());

                this->target_transport.send(temp_buffer, data_length);
            }

            temp_buffer += data_length;
            temp_length -= data_length;
        }
    }

public:
    virtual bool next() {
        this->reset_compressor = true;

        this->mppc_enc.~rdp_mppc_61_enc();

        new (&this->mppc_enc) rdp_mppc_61_enc_hash_based(this->verbose_compression);

        return this->target_transport.next();
    }

    virtual void timestamp(timeval now) {
        this->target_transport.timestamp(now);
    }
};  // class BulkCompressionOutTransport

#endif  // #ifndef REDEMPTION_TRANSPORT_BULK_COMPRESSION_TRANSPORT_HPP
