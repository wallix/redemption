/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once

#include "gdi/screen_info.hpp"
#include "transport/transport.hpp"
#include "capture/wrm_chunk_type.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

constexpr inline std::size_t WRM_HEADER_SIZE = 8;

inline void send_wrm_chunk(Transport& t, WrmChunkType chunktype, uint16_t data_size, uint16_t count)
{
    StaticOutStream<WRM_HEADER_SIZE> header;
    header.out_uint16_le(safe_int(chunktype));
    header.out_uint32_le(WRM_HEADER_SIZE + data_size);
    header.out_uint16_le(count);
    t.send(header.get_bytes());
}

struct WrmMetaChunk
{
    uint8_t version = 0;

    uint16_t width = 0;
    uint16_t height = 0;
    BitsPerPixel bpp {0};

    uint16_t cache_0_entries = 0;
    uint16_t cache_0_size = 0;
    uint16_t cache_1_entries = 0;
    uint16_t cache_1_size = 0;
    uint16_t cache_2_entries = 0;
    uint16_t cache_2_size = 0;

    uint16_t cache_3_entries = 0;
    uint16_t cache_3_size = 0;
    uint16_t cache_4_entries = 0;
    uint16_t cache_4_size = 0;

    uint16_t number_of_cache = 0;

    bool     use_waiting_list = false;

    bool     cache_0_persistent = false;
    bool     cache_1_persistent = false;
    bool     cache_2_persistent = false;

    bool     cache_3_persistent = false;
    bool     cache_4_persistent = false;

    WrmCompressionAlgorithm  compression_algorithm = WrmCompressionAlgorithm::no_compression;

    bool     remote_app = false;


    void send(Transport& t) const
    {
        StaticOutStream<48> payload;

        payload.out_uint16_le(safe_int(WrmChunkType::META_FILE));
        auto const pos = payload.get_offset();
        payload.out_skip_bytes(4);
        payload.out_uint16_le(1);

        payload.out_uint16_le(this->version);
        payload.out_uint16_le(this->width);
        payload.out_uint16_le(this->height);
        payload.out_uint16_le(safe_int(this->bpp));

        payload.out_uint16_le(this->cache_0_entries);
        payload.out_uint16_le(this->cache_0_size);
        payload.out_uint16_le(this->cache_1_entries);
        payload.out_uint16_le(this->cache_1_size);
        payload.out_uint16_le(this->cache_2_entries);
        payload.out_uint16_le(this->cache_2_size);

        if (this->version > 3) {
            payload.out_uint8(this->number_of_cache);
            payload.out_uint8(this->use_waiting_list);

            payload.out_uint8(this->cache_0_persistent);
            payload.out_uint8(this->cache_1_persistent);
            payload.out_uint8(this->cache_2_persistent);

            payload.out_uint16_le(this->cache_3_entries);
            payload.out_uint16_le(this->cache_3_size);
            payload.out_uint8(this->cache_3_persistent);
            payload.out_uint16_le(this->cache_4_entries);
            payload.out_uint16_le(this->cache_4_size);
            payload.out_uint8(this->cache_4_persistent);

            payload.out_uint8(static_cast<uint8_t>(this->compression_algorithm));

            if (this->version > 4) {
                payload.out_uint8(this->remote_app);
            }
        }

        payload.set_out_uint32_le(payload.get_offset(), pos);

        t.send(payload.get_bytes());
    }

    void receive(InStream& stream)
    {
        // reset values
        *this = WrmMetaChunk{};

        this->version                  = stream.in_uint16_le();
        this->width                    = stream.in_uint16_le();
        this->height                   = stream.in_uint16_le();
        this->bpp                      = checked_int(stream.in_uint16_le());
        this->cache_0_entries          = stream.in_uint16_le();
        this->cache_0_size             = stream.in_uint16_le();
        this->cache_1_entries          = stream.in_uint16_le();
        this->cache_1_size             = stream.in_uint16_le();
        this->cache_2_entries          = stream.in_uint16_le();
        this->cache_2_size             = stream.in_uint16_le();

        this->number_of_cache          = 3;

        if (this->version > 3) {
            this->number_of_cache      = stream.in_uint8();
            this->use_waiting_list     = bool(stream.in_uint8());

            this->cache_0_persistent   = bool(stream.in_uint8());
            this->cache_1_persistent   = bool(stream.in_uint8());
            this->cache_2_persistent   = bool(stream.in_uint8());

            this->cache_3_entries      = stream.in_uint16_le();
            this->cache_3_size         = stream.in_uint16_le();
            this->cache_3_persistent   = bool(stream.in_uint8());

            this->cache_4_entries      = stream.in_uint16_le();
            this->cache_4_size         = stream.in_uint16_le();
            this->cache_4_persistent   = bool(stream.in_uint8());

            this->compression_algorithm = static_cast<WrmCompressionAlgorithm>(stream.in_uint8());

            if (this->version > 4) {
                this->remote_app       = bool(stream.in_uint8());
            }

            assert(is_valid_enum_value(this->compression_algorithm));
            if (!is_valid_enum_value(this->compression_algorithm)) {
                this->compression_algorithm = WrmCompressionAlgorithm::no_compression;
            }
        }
    }
};
