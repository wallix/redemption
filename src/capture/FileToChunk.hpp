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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "FileToGraphic.hpp"
#include "transport/transport.hpp"
#include "RDPChunkedDevice.hpp"
#include "utils/compression_transport_wrapper.hpp"

class FileToChunk
{
    unsigned char stream_buf[65536];
    InStream stream;

    CompressionInTransportWrapper compression_wrapper;

    Transport * trans_source;
    Transport * trans;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;

    uint16_t nbconsumers;

    RDPChunkedDevice * consumers[10];

public:
    timeval record_now;

    bool meta_ok;

    uint16_t info_version;
    uint16_t info_width;
    uint16_t info_height;
    uint16_t info_bpp;
    uint16_t info_number_of_cache;
    bool     info_use_waiting_list;
    uint16_t info_cache_0_entries;
    uint16_t info_cache_0_size;
    bool     info_cache_0_persistent;
    uint16_t info_cache_1_entries;
    uint16_t info_cache_1_size;
    bool     info_cache_1_persistent;
    uint16_t info_cache_2_entries;
    uint16_t info_cache_2_size;
    bool     info_cache_2_persistent;
    uint16_t info_cache_3_entries;
    uint16_t info_cache_3_size;
    bool     info_cache_3_persistent;
    uint16_t info_cache_4_entries;
    uint16_t info_cache_4_size;
    bool     info_cache_4_persistent;
    WrmCompressionAlgorithm info_compression_algorithm;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        end_of_transport = 1,
    };

    FileToChunk(Transport * trans, Verbose verbose)
        : stream(this->stream_buf)
        , compression_wrapper(*trans, WrmCompressionAlgorithm::no_compression)
        , trans_source(trans)
        , trans(trans)
        // variables used to read batch of orders "chunks"
        , chunk_size(0)
        , chunk_type(0)
        , chunk_count(0)
        , nbconsumers(0)
        , consumers()
        , meta_ok(false)
        , info_version(0)
        , info_width(0)
        , info_height(0)
        , info_bpp(0)
        , info_number_of_cache(0)
        , info_use_waiting_list(true)
        , info_cache_0_entries(0)
        , info_cache_0_size(0)
        , info_cache_0_persistent(false)
        , info_cache_1_entries(0)
        , info_cache_1_size(0)
        , info_cache_1_persistent(false)
        , info_cache_2_entries(0)
        , info_cache_2_size(0)
        , info_cache_2_persistent(false)
        , info_cache_3_entries(0)
        , info_cache_3_size(0)
        , info_cache_3_persistent(false)
        , info_cache_4_entries(0)
        , info_cache_4_size(0)
        , info_cache_4_persistent(false)
        , info_compression_algorithm(WrmCompressionAlgorithm::no_compression)
        , verbose(verbose)
    {
        while (this->next_chunk()) {
            this->interpret_chunk();
            if (this->meta_ok) {
                break;
            }
        }
    }

    void add_consumer(RDPChunkedDevice * chunk_device) {
        REDASSERT(nbconsumers < (sizeof(consumers) / sizeof(consumers[0]) - 1));
        this->consumers[this->nbconsumers++] = chunk_device;
    }

    bool next_chunk() {
        try {
            {
                auto const buf_sz = FileToGraphic::HEADER_SIZE;
                unsigned char buf[buf_sz];
                auto * p = buf;
                this->trans->recv(&p, buf_sz);
                InStream header(buf);
                this->chunk_type  = header.in_uint16_le();
                this->chunk_size  = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
            }

            if (this->chunk_size > 65536) {
                LOG(LOG_INFO,"chunk_size (%d) > 65536", this->chunk_size);
                return false;
            }
            this->stream = InStream(this->stream_buf, 0);   // empty stream
            if (this->chunk_size - FileToGraphic::HEADER_SIZE > 0) {
                auto * p = this->stream_buf;
                this->trans->recv(&p, this->chunk_size - FileToGraphic::HEADER_SIZE);
                this->stream = InStream(this->stream_buf, p - this->stream_buf);
            }
        }
        catch (Error const & e) {
            if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                throw;
            }

            if (this->verbose) {
                LOG(LOG_INFO, "receive error %u : end of transport", e.id);
            }
            // receive error, end of transport
            return false;
        }

        return true;
    }

    void interpret_chunk() {
        switch (this->chunk_type) {
        case META_FILE:
            this->info_version                   = this->stream.in_uint16_le();
            this->info_width                     = this->stream.in_uint16_le();
            this->info_height                    = this->stream.in_uint16_le();
            this->info_bpp                       = this->stream.in_uint16_le();
            this->info_cache_0_entries           = this->stream.in_uint16_le();
            this->info_cache_0_size              = this->stream.in_uint16_le();
            this->info_cache_1_entries           = this->stream.in_uint16_le();
            this->info_cache_1_size              = this->stream.in_uint16_le();
            this->info_cache_2_entries           = this->stream.in_uint16_le();
            this->info_cache_2_size              = this->stream.in_uint16_le();

            if (this->info_version <= 3) {
                this->info_number_of_cache       = 3;
                this->info_use_waiting_list      = false;

                this->info_cache_0_persistent    = false;
                this->info_cache_1_persistent    = false;
                this->info_cache_2_persistent    = false;
            }
            else {
                this->info_number_of_cache       = this->stream.in_uint8();
                this->info_use_waiting_list      = (this->stream.in_uint8() ? true : false);

                this->info_cache_0_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_1_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_2_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_3_entries       = this->stream.in_uint16_le();
                this->info_cache_3_size          = this->stream.in_uint16_le();
                this->info_cache_3_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_4_entries       = this->stream.in_uint16_le();
                this->info_cache_4_size          = this->stream.in_uint16_le();
                this->info_cache_4_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_compression_algorithm = static_cast<WrmCompressionAlgorithm>(this->stream.in_uint8());
                REDASSERT(is_valid_enum_value(this->info_compression_algorithm));
                if (!is_valid_enum_value(this->info_compression_algorithm)) {
                    this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;
                }

                // re-init
                this->compression_wrapper.~CompressionTransportWrapper();
                new (&this->compression_wrapper) CompressionInTransportWrapper(
                    *this->trans_source, this->info_compression_algorithm);
                this->trans = &this->compression_wrapper.get();
            }

            this->stream.rewind();

            if (!this->meta_ok) {
                this->meta_ok = true;
            }
            break;
        case RESET_CHUNK:
            this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;

            this->trans = this->trans_source;
            break;
        }

        for (size_t i = 0; i < this->nbconsumers ; i++) {
            if (this->consumers[i]) {
                this->consumers[i]->chunk(this->chunk_type, this->chunk_count, this->stream.clone());
            }
        }
    }   // void interpret_chunk()

    void play(bool const & requested_to_stop) {
        while (!requested_to_stop && this->next_chunk()) {
            this->interpret_chunk();
        }
    }
};

