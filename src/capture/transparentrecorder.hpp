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

#include "core/channel_names.hpp"
#include "transport/transport.hpp"
#include "capture/transparentchunk.hpp"
#include "utils/stream.hpp"
#include "utils/difftimeval.hpp"

class TransparentRecorder {
private:
    Transport * t;

public:
    explicit TransparentRecorder(Transport * t) : t(t) {
        this->send_meta_chunk();
    }

    void send_data_indication_ex(uint16_t channelId, uint8_t const * packet_data, std::size_t packet_size) {
        constexpr unsigned payload_size = 2;
        StaticOutStream<TRANSPARENT_CHUNK_HEADER_SIZE + payload_size> header;

        this->make_chunk_header(header, CHUNK_TYPE_SLOWPATH, payload_size + packet_size);
        header.out_uint16_le(channelId);

        assert(header.get_offset() == header.get_capacity());
        this->t->send(header.get_data(), header.get_offset());
        this->t->send(packet_data, packet_size);
    }

    void send_fastpath_data(InStream & data) {
        StaticOutStream<TRANSPARENT_CHUNK_HEADER_SIZE> header;
        this->make_chunk_header(header, CHUNK_TYPE_FASTPATH, data.get_capacity());

        assert(header.get_offset() == header.get_capacity());
        this->t->send(header.get_data(), header.get_capacity());
        this->t->send(data.get_data(), data.get_capacity());
    }

    void send_to_front_channel( CHANNELS::ChannelNameId mod_channel_name
                              , uint8_t const * data, size_t length
                              , size_t chunk_size, int flags) {
        constexpr unsigned payload_size = 9;
        StaticOutStream<TRANSPARENT_CHUNK_HEADER_SIZE + payload_size> header;
        uint8_t mod_channel_name_length = strlen(mod_channel_name.c_str());

        this->make_chunk_header(header, CHUNK_TYPE_FRONTCHANNEL, payload_size + mod_channel_name_length + length);
        header.out_uint8(mod_channel_name_length);
        header.out_uint16_le(length);
        header.out_uint16_le(chunk_size);
        header.out_uint32_le(flags);

        assert(header.get_offset() == header.get_capacity());
        this->t->send(header.get_data(), header.get_offset());
        this->t->send(mod_channel_name.c_str(), mod_channel_name_length);
        this->t->send(data, length);
    }

    void server_resize(uint16_t width, uint16_t height, uint8_t bpp) {
        constexpr unsigned payload_size = 5;
        StaticOutStream<TRANSPARENT_CHUNK_HEADER_SIZE + payload_size> header;

        this->make_chunk_header(header, CHUNK_TYPE_RESIZE, payload_size);
        header.out_uint16_le(width);
        header.out_uint16_le(height);
        header.out_uint8(bpp);

        assert(header.get_offset() == header.get_capacity());
        this->t->send(header.get_data(), header.get_offset());
    }

private:
    void make_chunk_header(OutStream & stream, uint8_t chunk_type, uint16_t data_size) {
        stream.out_uint8(chunk_type);
        stream.out_uint16_le(data_size);
        stream.out_timeval_to_uint64le_usec(tvtime());
    }

    void send_meta_chunk() {
        const uint8_t trm_format_version = 0;

        constexpr unsigned payload_size = 1;
        StaticOutStream<TRANSPARENT_CHUNK_HEADER_SIZE + payload_size> header;

        this->make_chunk_header(header, CHUNK_TYPE_META, payload_size);
        header.out_uint8(trm_format_version);

        assert(header.get_offset() == header.get_capacity());
        this->t->send(header.get_data(), header.get_offset());
    }
};

