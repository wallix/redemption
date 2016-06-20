/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/


#pragma once

#include "utils/stream.hpp"
#include "transport/transport.hpp"
#include "wrm_label.hpp"


inline void send_wrm_chunk(Transport & t, uint16_t chunktype, uint16_t data_size, uint16_t count)
{
    StaticOutStream<8> header;
    header.out_uint16_le(chunktype);
    header.out_uint32_le(8 + data_size);
    header.out_uint16_le(count);
    t.send(header.get_data(), header.get_offset());
}


inline void send_meta_chunk(
    Transport & t
  , uint8_t wrm_format_version

  , uint16_t info_width
  , uint16_t info_height
  , uint16_t info_bpp

  , uint16_t info_cache_0_entries
  , uint16_t info_cache_0_size
  , uint16_t info_cache_1_entries
  , uint16_t info_cache_1_size
  , uint16_t info_cache_2_entries
  , uint16_t info_cache_2_size

  , uint16_t info_number_of_cache
  , bool     info_use_waiting_list

  , bool     info_cache_0_persistent
  , bool     info_cache_1_persistent
  , bool     info_cache_2_persistent

  , uint16_t info_cache_3_entries
  , uint16_t info_cache_3_size
  , bool     info_cache_3_persistent
  , uint16_t info_cache_4_entries
  , uint16_t info_cache_4_size
  , bool     info_cache_4_persistent

  , uint8_t  index_algorithm
) {
    StaticOutStream<36> payload;
    payload.out_uint16_le(wrm_format_version);
    payload.out_uint16_le(info_width);
    payload.out_uint16_le(info_height);
    payload.out_uint16_le(info_bpp);

    payload.out_uint16_le(info_cache_0_entries);
    payload.out_uint16_le(info_cache_0_size);
    payload.out_uint16_le(info_cache_1_entries);
    payload.out_uint16_le(info_cache_1_size);
    payload.out_uint16_le(info_cache_2_entries);
    payload.out_uint16_le(info_cache_2_size);

    if (wrm_format_version > 3) {
        payload.out_uint8(info_number_of_cache);
        payload.out_uint8(info_use_waiting_list);

        payload.out_uint8(info_cache_0_persistent);
        payload.out_uint8(info_cache_1_persistent);
        payload.out_uint8(info_cache_2_persistent);

        payload.out_uint16_le(info_cache_3_entries);
        payload.out_uint16_le(info_cache_3_size);
        payload.out_uint8(info_cache_3_persistent);
        payload.out_uint16_le(info_cache_4_entries);
        payload.out_uint16_le(info_cache_4_size);
        payload.out_uint8(info_cache_4_persistent);

        payload.out_uint8(index_algorithm);
    }

    send_wrm_chunk(t, META_FILE, payload.get_offset(), 1);
    t.send(payload.get_data(), payload.get_offset());
}

