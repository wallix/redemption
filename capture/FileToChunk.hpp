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

#ifndef _REDEMPTION_CAPTURE_FILETOCHUNK_HPP_
#define _REDEMPTION_CAPTURE_FILETOCHUNK_HPP_

#include "FileToGraphic.hpp"
#include "transport.hpp"
#include "RDPChunkedDevice.hpp"

struct FileToChunk {
    BStream stream;

    Transport * trans_source;
    Transport * trans;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;

public:
    uint16_t nbconsumers;

    struct RDPChunkedDevice * consumers[10];

    bool meta_ok;

    uint32_t verbose;

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
    uint8_t  info_compression_algorithm;

    GZipCompressionInTransport   gzcit;
    //LzmaCompressionInTransport   lcit;
    SnappyCompressionInTransport scit;

    FileToChunk(Transport * trans, uint32_t verbose)
        : stream(65536)
        , trans_source(trans)
        , trans(trans)
        // variables used to read batch of orders "chunks"
        , chunk_size(0)
        , chunk_type(0)
        , chunk_count(0)
        , nbconsumers(0)
        , consumers()
        , meta_ok(false)
        , verbose(verbose)
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
        , info_compression_algorithm(0)
        , gzcit(*trans)
        //, lcit(*trans, verbose)
        , scit(*trans) {
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
            BStream header(FileToGraphic::HEADER_SIZE);
            this->trans->recv(&header.end, FileToGraphic::HEADER_SIZE);
            this->chunk_type  = header.in_uint16_le();
            this->chunk_size  = header.in_uint32_le();
            this->chunk_count = header.in_uint16_le();

            if (this->chunk_size > 65536) {
                LOG(LOG_INFO,"chunk_size (%d) > 65536", this->chunk_size);
                return false;
            }
            this->stream.reset();
            if (this->chunk_size - FileToGraphic::HEADER_SIZE > 0) {
                this->trans->recv(&this->stream.end, this->chunk_size - FileToGraphic::HEADER_SIZE);
            }
        }
        catch (Error & e) {
            if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                throw;
            }

            LOG(LOG_INFO,"receive error %u : end of transport", e.id);
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

                this->info_compression_algorithm = this->stream.in_uint8();
                //REDASSERT(this->info_compression_algorithm < 4);
                REDASSERT(this->info_compression_algorithm < 3);

                switch (this->info_compression_algorithm) {
                case 1:
                    this->trans = &this->gzcit;
                    break;
                case 2:
                    this->trans = &this->scit;
                    break;
                //case 3:
                //    this->trans = &this->lcit;
                //    break;
                default:
                    this->trans = this->trans_source;
                    break;
                }
            }

            this->stream.p = this->stream.end;

            this->meta_ok = true;
            break;
        case RESET_CHUNK:
            this->info_compression_algorithm = 0;

            this->trans = this->trans_source;
            break;
        }

        for (size_t i = 0; i < this->nbconsumers ; i++) {
            if (this->consumers[i]) {
                this->consumers[i]->chunk(this->chunk_type, this->chunk_count, this->stream);
            }
        }
    }   // void interpret_chunk()

    void play() {
        while (this->next_chunk()) {
            this->interpret_chunk();
        }
    }
};

#endif
